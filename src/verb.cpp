module openas;

import std;

namespace openas {

namespace {

constexpr std::array<std::string_view, 4> effects{"read", "reversible", "irreversible", "external"};

}  // namespace

std::string_view name(Effect e) noexcept { return effects[static_cast<std::size_t>(e)]; }

std::optional<Effect> parse_effect(std::string_view s) noexcept {
    for (std::size_t i = 0; i < effects.size(); ++i)
        if (effects[i] == s) return static_cast<Effect>(i);
    return std::nullopt;
}

Effect read_effect(std::string_view s) noexcept { return parse_effect(s).value_or(Effect::external); }

}  // namespace openas
