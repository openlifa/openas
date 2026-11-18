// openas:envelope — how a run is reported (SPEC.md §6): the kinds of record,
// the exit codes of the final `result`, and the codes of an `error`.
export module openas:envelope;

import std;

export namespace openas {

enum class RecordKind : std::uint8_t { phase, progress, log, notice, error, data, result };

[[nodiscard]] std::string_view name(RecordKind k) noexcept;
[[nodiscard]] std::optional<RecordKind> parse_record_kind(std::string_view s) noexcept;

namespace exit_code {
inline constexpr int done         = 0;
inline constexpr int failed       = 1;
inline constexpr int needs_answer = 2;
inline constexpr int not_ready    = 3;
}  // namespace exit_code

namespace error_code {
inline constexpr std::string_view needs_answer     = "needs-answer";
inline constexpr std::string_view refused          = "refused";
inline constexpr std::string_view not_found        = "not-found";
inline constexpr std::string_view invalid_argument = "invalid-argument";
inline constexpr std::string_view stale            = "stale";
inline constexpr std::string_view timeout          = "timeout";
inline constexpr std::string_view failed           = "failed";
inline constexpr std::string_view internal         = "internal";
}  // namespace error_code

// An error code as a reader must take it: one it does not know is `failed` (§6.3).
[[nodiscard]] std::string_view read_error_code(std::string_view code) noexcept;

}  // namespace openas
