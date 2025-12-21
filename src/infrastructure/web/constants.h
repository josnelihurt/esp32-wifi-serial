#pragma once

namespace jrb::wifi_serial::http {
enum class StatusCode {
  OK = 200,
  BAD_REQUEST = 400,
  UNAUTHORIZED = 401,
  NOT_FOUND = 404,
  INTERNAL_SERVER_ERROR = 500,
};
constexpr int toInt(StatusCode code) { return static_cast<int>(code); }

enum class mime {
  TEXT_PLAIN,
  APPLICATION_JSON,
  TEXT_HTML,
};

constexpr const char *toString(mime mime) {
  switch (mime) {
  case mime::TEXT_PLAIN:
    return "text/plain";
  case mime::APPLICATION_JSON:
    return "application/json";
  case mime::TEXT_HTML:
    return "text/html";
  default:
    break;
  }
  return "Unknown mime type";
}
} // namespace jrb::wifi_serial::http