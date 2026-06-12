#ifndef CHECK_EDGE_HXX
#define CHECK_EDGE_HXX

#include "acis.hxx"
#include "cstrapi.hxx"
#include "insanity_list.hxx"
#include "chk_stat.hxx"

enum edge_check_status {
    EDGE_CHECK_OK                    = 0,
    EDGE_CHECK_NULL_EDGE             = 1 << 0,
    EDGE_CHECK_NULL_CURVE            = 1 << 1,
    EDGE_CHECK_NULL_VERTEX           = 1 << 2,
    EDGE_CHECK_DEGENERATE            = 1 << 3,
    EDGE_CHECK_BAD_PARAM_RANGE       = 1 << 4,
    EDGE_CHECK_VERTEX_NOT_ON_CURVE   = 1 << 5,
    EDGE_CHECK_BAD_CLOSURE           = 1 << 6,
    EDGE_CHECK_COEDGE_SENSE_ERROR    = 1 << 7,
    EDGE_CHECK_EVAL_FAILURE          = 1 << 8,
    EDGE_CHECK_NAN_COORDINATES       = 1 << 9,
    EDGE_CHECK_BAD_FIT_TOLERANCE     = 1 << 10,
    EDGE_CHECK_BAD_LENGTH            = 1 << 11,
    EDGE_CHECK_NON_G1_CONTINUITY     = 1 << 12,
    EDGE_CHECK_BAD_BOUNDING_BOX      = 1 << 13,
    EDGE_CHECK_BAD_PARAM_NORMALIZATION = 1 << 14
};

class edge_check_result {
public:
    edge_check_result();
    ~edge_check_result();

    int get_status() const;
    bool is_ok() const;
    void set_status(int status);

    int eval_failure_count() const;

    void add_insanity(insanity_data *data);
    insanity_list *get_insanity_list();

private:
    int          _status;
    int          _eval_failure_count;
    insanity_list _insanities;
};

outcome api_check_edge_errors(
    EDGE              *edge,
    edge_check_result &result,
    AcisOptions       *ao = NULL
);

logical check_edge_null(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_curve(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_vertices(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_degenerate(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_parameter_range(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_vertex_on_curve(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_closure(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_coedge_sense(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_evaluation(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_fit_tolerance(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_length(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_g1_continuity(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_bounding_box(
    EDGE          *edge,
    insanity_list *ilist
);

logical check_edge_param_normalization(
    EDGE          *edge,
    insanity_list *ilist
);

int api_check_edge(
    EDGE *edge,
    int  *insanity_count = NULL
);

#endif
