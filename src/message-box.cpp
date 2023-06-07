#include "phodispl/box.hpp"
#include "phodispl/config.hpp"
#include "phodispl/message-box.hpp"
#include "phodispl/viewport.hpp"

#include <pixglot/exception.hpp>



void message_box::render(
    const std::string& heading,
    const std::string& body,
    float              alpha
) const {
  auto heading_u32 = viewport::convert_string(heading);
  auto body_u32    = viewport::convert_string(body);

  auto hs = global_config().theme_heading_size;
  auto ts = global_config().theme_text_size;

  auto [w1, h1] = viewport_->string_dimensions(heading_u32, hs);
  auto [w2, h2] = viewport_->string_dimensions(body_u32,    ts);

  float w = std::max<float>(w1, w2);
  float h = h1 + h2 + ts;

  box ruler = {
    .width        = w,
    .height       = 2.f,
    .x            = 0.f,
    .y            = 0.5f * h - 55.f,
    .anchor       = placement::center,
    .align_pixels = true
  };

  auto head = global_config().theme_heading_color;

  head[0] *= alpha;
  head[1] *= alpha;
  head[2] *= alpha;
  head[3] *= alpha;

  viewport_->draw_string(heading_u32, hs, -0.5 * w, 0.5 * h  - hs, head);
  viewport_->draw_string(body_u32,    ts, -0.5 * w, 0.5 * h  - 4.2 * ts,
                          global_config().theme_text_color);

  head[0] *= 0.3;
  head[1] *= 0.3;
  head[2] *= 0.3;
  head[3] *= 0.3;
  viewport_->render_solid(ruler, head);
}





namespace {
  [[nodiscard]] std::string bump_first(std::string_view in) {
    std::string out{in};
    if (!out.empty()) {
      out[0] = toupper(out[0]);
    }
    return out;
  }



  struct message {
    std::string header;
    std::string body;
  };



  [[nodiscard]] message format_no_stream_access(
      const pixglot::no_stream_access& /*err*/
  ) {
    return {
      .header{"[404]  Data Not Found"},
      .body  {"Cannot access input data."}
    };
  }



  [[nodiscard]] message format_no_decoder(const pixglot::no_decoder& /*err*/) {
    return {
      .header{"[415]  Unsupported Media Type"},
      .body  {"None of the decoders recognized this file type."}
    };
  }



  [[nodiscard]] message format_decode_error(const pixglot::decode_error& err) {
    return {
      .header{"[409]  Failed to Decode"},
      .body  {"The " + pixglot::to_string(err.decoder()) +
        " decoder failed to decode the input:\n" +
        bump_first(err.plain())}
    };
  }



  [[nodiscard]] message format_generic_error(const pixglot::base_exception& err) {
    return {
      .header{"[500]  Internal Error"},
      .body  {std::string{err.message()}}
    };
  }



  [[nodiscard]] message format_base_error(const pixglot::base_exception& error) {
    if (const auto* err = dynamic_cast<const pixglot::no_stream_access*>(&error)) {
      return format_no_stream_access(*err);
    }
    if (const auto* err = dynamic_cast<const pixglot::no_decoder*>(&error)) {
      return format_no_decoder(*err);
    }
    if (const auto* err = dynamic_cast<const pixglot::decode_error*>(&error)) {
      return format_decode_error(*err);
    }
    return format_generic_error(error);
  }
}



void message_box::render(
    const pixglot::base_exception&        error,
    const std::optional<std::filesystem::path>& path,
    float                                       alpha
) const {
  auto [header, body] = format_base_error(error);

  if (path) {
    body += "\n\nFile: " + path->string();
  }

  render(header, body, alpha);
}
