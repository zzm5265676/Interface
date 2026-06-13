#ifndef CHECK_SURFACE_HXX
#define CHECK_SURFACE_HXX

#include "acis.hxx"
#include "cstrapi.hxx"
#include "insanity_list.hxx"
#include "chk_stat.hxx"

enum surface_check_status {
    SURF_CHECK_OK                    = 0,
    SURF_CHECK_NULL_SURFACE          = 1 << 0,
    SURF_CHECK_EVAL_FAILURE          = 1 << 1,
    SURF_CHECK_NAN_COORDINATES       = 1 << 2,
    SURF_CHECK_BAD_PARAMETER_RANGE   = 1 << 3,
    SURF_CHECK_SELF_INTERSECT        = 1 << 4,
    SURF_CHECK_BAD_CLOSURE           = 1 << 5,
    SURF_CHECK_NON_G0                = 1 << 6,
    SURF_CHECK_NON_G1                = 1 << 7,
    SURF_CHECK_BAD_FIT_TOLERANCE     = 1 << 8,
    SURF_CHECK_BAD_SINGULARITY       = 1 << 9,
    SURF_CHECK_ILLEGAL_SURFACE       = 1 << 10,
    SURF_CHECK_BAD_NORMAL            = 1 << 11,
    SURF_CHECK_NON_G2                = 1 << 12,
    SURF_CHECK_BAD_UV_COORDINATES    = 1 << 13,
    SURF_CHECK_DEGENERATE_AREA       = 1 << 14,
    SURF_CHECK_BAD_PERIODICITY       = 1 << 15
};

class surface_check_result {
public:
    surface_check_result();
    ~surface_check_result();

    int get_status() const;
    bool is_ok() const;
    void set_status(int status);

    int eval_failure_count() const;
    int singularity_count() const;
    void note_eval_failure();
    void note_singularity();

    void add_insanity(insanity_data *data);
    insanity_list *get_insanity_list();

private:
    int          _status;
    int          _eval_failure_count;
    int          _singularity_count;
    insanity_list _insanities;
};

outcome api_check_surface_ok(
    SURFACE             *surface,
    surface_check_result &result,
    AcisOptions         *ao = NULL
);

logical check_surface_null(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_evaluation(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_parameter_range(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_continuity(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_singularity(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_closure(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_fit_tolerance(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_bspline_surface(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_self_intersection(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_normal_consistency(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_g2_continuity(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_uv_coordinates(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_area_degenerate(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

logical check_surface_periodicity(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status = NULL
);

int check_surface_ok(
    SURFACE *surface,
    int     *insanity_count = NULL
);

#endif
