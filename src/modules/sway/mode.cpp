#include "modules/sway/mode.hpp"
#include <spdlog/spdlog.h>

namespace waybar::modules::sway {

Mode::Mode(const std::string& id, const Json::Value& config) : ALabel(config, "mode", id, "{}") {
  ipc_.subscribe(R"(["mode"])");
  ipc_.signal_event.connect(sigc::mem_fun(*this, &Mode::onEvent));
  // Launch worker
  worker();
  dp.emit();
}

void Mode::onEvent(const struct Ipc::ipc_response& res) {
  try {
    auto payload = parser_.parse(res.payload);
    if (payload["change"] != "default") {
      mode_ = Glib::Markup::escape_text(payload["change"].asString());
    } else {
      mode_.clear();
    }
    dp.emit();
  } catch (const std::exception& e) {
    spdlog::error("Mode: {}", e.what());
  }
}

void Mode::worker() {
  thread_ = [this] {
    try {
      ipc_.handleEvent();
    } catch (const std::exception& e) {
      spdlog::error("Mode: {}", e.what());
    }
  };
}

auto Mode::update() -> void {
  if (mode_.empty()) {
    event_box_.hide();
  } else {
    label_.set_markup(fmt::format(format_, mode_));
    if (tooltipEnabled()) {
      label_.set_tooltip_text(mode_);
    }
    event_box_.show();
  }
}

}  // namespace waybar::modules::sway
