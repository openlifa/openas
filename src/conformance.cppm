// openas.conformance — what a system's description of its verbs, an actor
// and a grant must hold (SPEC.md §7). Each failure names its clause.
export module openas.conformance;

import std;
import openas;

export namespace openas::conformance {

struct Report {
    int                      held = 0;
    std::vector<std::string> failed;  // "7.2 rm: names are unique"

    [[nodiscard]] bool ok() const noexcept { return failed.empty(); }
};

[[nodiscard]] Report check(std::span<const Verb> verbs);
[[nodiscard]] Report check(const Actor& actor);
[[nodiscard]] Report check(const Grant& grant);

}  // namespace openas::conformance
