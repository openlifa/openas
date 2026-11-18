// openas:verb — effect classes and the description of a verb (SPEC.md §3, §4).
export module openas:verb;

import std;

export namespace openas {

// What running a verb does to the world, in increasing order (§3.1).
enum class Effect : std::uint8_t {
    read,          // changes nothing
    reversible,    // changes what the system can take back
    irreversible,  // removes or overwrites what cannot be taken back
    external,      // reaches outside the system: the host, the network, a device
};

[[nodiscard]] std::string_view name(Effect e) noexcept;
[[nodiscard]] std::optional<Effect> parse_effect(std::string_view s) noexcept;
// An effect as a reader must take it: one it does not know is `external` (§3.1).
[[nodiscard]] Effect read_effect(std::string_view s) noexcept;

// Whether a verb of this effect asks before it runs (§3.2).
[[nodiscard]] constexpr bool asks_first(Effect e) noexcept { return e >= Effect::irreversible; }

struct Param {
    std::string name;
    std::string description;
    bool        required = false;
    bool        repeated = false;
    bool        flag = false;  // an option without a value
};

// One verb a system offers (§4).
struct Verb {
    std::string              name;  // lower case words joined by '-' or '.'
    std::string              summary;
    Effect                   effect = Effect::read;
    bool                     person = false;  // the person's own (§3.4)
    std::vector<Param>       arguments;       // by position
    std::vector<Param>       options;         // by name
    std::vector<std::string> answers;         // the options that answer its questions
};

}  // namespace openas
