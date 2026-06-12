#ifndef BS3_CURVE_CHECK_HXX
#define BS3_CURVE_CHECK_HXX

#include "acis.hxx"
#include "cstrapi.hxx"
#include "insanity_list.hxx"
#include "chk_stat.hxx"

enum bs3_curve_check_status {
    BS3_CURVE_CHECK_OK                = 0,
    BS3_CURVE_CHECK_NULL_CURVE        = 1 << 0,
    BS3_CURVE_CHECK_BAD_ORDER         = 1 << 1,
    BS3_CURVE_CHECK_BAD_KNOT_VECTOR   = 1 << 2,
    BS3_CURVE_CHECK_BAD_CP_COUNT      = 1 << 3,
    BS3_CURVE_CHECK_COINCIDENT_CPS    = 1 << 4,
    BS3_CURVE_CHECK_EVAL_FAILURE      = 1 << 5,
    BS3_CURVE_CHECK_NAN_COORDINATES   = 1 << 6,
    BS3_CURVE_CHECK_BAD_PARAM_RANGE   = 1 << 7,
    BS3_CURVE_CHECK_BAD_CLOSURE       = 1 << 8,
    BS3_CURVE_CHECK_BAD_FIT_TOL       = 1 << 9,
    BS3_CURVE_CHECK_BAD_KNOT_MULT     = 1 << 10,
    BS3_CURVE_CHECK_DEGENERATE        = 1 << 11,
    BS3_CURVE_CHECK_BAD_CONVEX_HULL   = 1 << 12,
    BS3_CURVE_CHECK_BAD_VD_PROPERTY   = 1 << 13,
    BS3_CURVE_CHECK_BAD_BOUNDING_BOX  = 1 << 14,
    BS3_CURVE_CHECK_BAD_ARC_LENGTH    = 1 << 15
};

class bs3_curve_check_result {
public:
    bs3_curve_check_result();
    ~bs3_curve_check_result();

    int get_status() const;
    bool is_ok() const;
    void set_status(int status);

    int eval_failure_count() const;
    int degenerate_count() const;
    void note_eval_failure();
    void note_degenerate();

    void add_insanity(insanity_data *data);
    insanity_list *get_insanity_list();

private:
    int          _status;
    int          _eval_failure_count;
    int          _degenerate_count;
    insanity_list _insanities;
};

outcome api_bs3_curve_check(
    BS3_CURVE            *curve,
    bs3_curve_check_result &result,
    AcisOptions          *ao = NULL
);

logical check_bs3_curve_null(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_order(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_control_points(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_knot_vector(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_evaluation(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_parameter_range(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_closure(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_fit_tolerance(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_degeneracy(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_derivatives(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_knot_multiplicity(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_convex_hull(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_vd_property(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_bounding_box(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

logical check_bs3_curve_arc_length(
    BS3_CURVE       *curve,
    insanity_list   *ilist
);

int bs3_curve_check(
    BS3_CURVE *curve,
    int       *insanity_count = NULL
);

#endif
