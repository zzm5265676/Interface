#ifndef CHECK_VERTEX_HXX
#define CHECK_VERTEX_HXX

#include "acis.hxx"
#include "cstrapi.hxx"
#include "insanity_list.hxx"
#include "chk_stat.hxx"

enum vertex_check_status {
    VTX_CHECK_OK                    = 0,
    VTX_CHECK_NULL_POINT            = 1 << 0,
    VTX_CHECK_NO_EDGES              = 1 << 1,
    VTX_CHECK_DEGENERATE_EDGE       = 1 << 2,
    VTX_CHECK_BAD_EDGE_CURVE        = 1 << 3,
    VTX_CHECK_EDGE_SENSE_MISMATCH   = 1 << 4,
    VTX_CHECK_NON_MANIFOLD          = 1 << 5,
    VTX_CHECK_COINCIDENT_VERTICES   = 1 << 6,
    VTX_CHECK_POINT_NOT_ON_CURVE    = 1 << 7,
    VTX_CHECK_BAD_BOUNDING_BOX      = 1 << 8,
    VTX_CHECK_BAD_NORMAL_CONSISTENCY= 1 << 9,
    VTX_CHECK_BAD_TOLERANCE         = 1 << 10,
    VTX_CHECK_SHARP_ANGLE           = 1 << 11
};

class vertex_check_result {
public:
    vertex_check_result();
    ~vertex_check_result();

    int get_status() const;
    bool is_ok() const;
    void set_status(int status);

    int edge_count() const;
    int bad_edge_count() const;
    int non_manifold_count() const;
    void note_edge();
    void note_bad_edge();
    void note_non_manifold();

    void add_insanity(insanity_data *data);
    insanity_list *get_insanity_list();

private:
    int          _status;
    int          _edge_count;
    int          _bad_edge_count;
    int          _non_manifold_count;
    insanity_list _insanities;
};

outcome api_check_vertex_errors(
    VERTEX            *vertex,
    vertex_check_result &result,
    AcisOptions       *ao = NULL
);

logical check_vertex_point_valid(
    VERTEX        *vertex,
    insanity_list *ilist
);

logical check_vertex_edges_valid(
    VERTEX        *vertex,
    insanity_list *ilist
);

logical check_vertex_edge_curves(
    VERTEX        *vertex,
    insanity_list *ilist
);

logical check_vertex_coincident(
    VERTEX        *vertex,
    insanity_list *ilist
);

logical check_vertex_edge_sense(
    VERTEX        *vertex,
    insanity_list *ilist
);

logical check_vertex_manifold(
    VERTEX        *vertex,
    insanity_list *ilist
);

logical check_vertex_bounding_box(
    VERTEX        *vertex,
    insanity_list *ilist
);

logical check_vertex_normal_consistency(
    VERTEX        *vertex,
    insanity_list *ilist
);

logical check_vertex_tolerance(
    VERTEX        *vertex,
    insanity_list *ilist
);

logical check_vertex_sharp_angle(
    VERTEX        *vertex,
    insanity_list *ilist
);

int api_check_vertex(
    VERTEX *vertex,
    int    *insanity_count = NULL
);

#endif
