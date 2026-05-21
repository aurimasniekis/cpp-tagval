#pragma once

#include <tagval/tagval.hpp>

class Kind : public tagval::OpenEnded<"split_kind", Kind> {
public:
    using base_t = OpenEnded;
    using base_t::base_t;

    TAGVAL_ENTRY(Kind, Builtin, builtin, "Built-in")

    using values_t = tagval::Values<Builtin>;
};
