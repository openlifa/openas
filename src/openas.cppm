// openas — actors and what they do with a system (SPEC.md): who did it and for
// whom, what kind of effect a verb has, who may run it without asking, and how
// a run is reported. Declarations and their text forms; standard library only.
export module openas;

export import :actor;
export import :verb;
export import :grant;
export import :envelope;

export namespace openas {

// The version of SPEC.md these declarations are.
inline constexpr std::string_view version = "0.2";

}  // namespace openas
