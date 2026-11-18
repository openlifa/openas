module openas;

import std;

namespace openas {

namespace {

constexpr std::array<std::string_view, 7> record_kinds{"phase", "progress", "log", "notice", "error", "data", "result"};
constexpr std::array<std::string_view, 8> error_codes{
    error_code::needs_answer, error_code::refused, error_code::not_found, error_code::invalid_argument,
    error_code::stale,        error_code::timeout, error_code::failed,    error_code::internal,
};

}  // namespace

std::string_view name(RecordKind k) noexcept { return record_kinds[static_cast<std::size_t>(k)]; }

std::optional<RecordKind> parse_record_kind(std::string_view s) noexcept {
    for (std::size_t i = 0; i < record_kinds.size(); ++i)
        if (record_kinds[i] == s) return static_cast<RecordKind>(i);
    return std::nullopt;
}

std::string_view read_error_code(std::string_view code) noexcept {
    for (std::string_view c : error_codes)
        if (c == code) return c;
    return error_code::failed;
}

}  // namespace openas
