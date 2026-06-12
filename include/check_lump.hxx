#ifndef CHECK_LUMP_HXX
#define CHECK_LUMP_HXX

#include "acis.hxx"
#include "cstrapi.hxx"
#include "insanity_list.hxx"
#include "chk_stat.hxx"

enum lump_check_status {
    LUMP_CHECK_OK                   = 0,
    LUMP_CHECK_NO_SHELL             = 1 << 0,
    LUMP_CHECK_EMPTY_SHELL          = 1 << 1,
    LUMP_CHECK_SHELL_SELF_INT       = 1 << 2,
    LUMP_CHECK_BAD_CONTAINMENT      = 1 << 3,
    LUMP_CHECK_INTERSECT_SHELLS     = 1 << 4,
    LUMP_CHECK_DEGENERATE_FACE      = 1 << 5,
    LUMP_CHECK_BAD_COEDGE_SENSE     = 1 << 6,
    LUMP_CHECK_NULL_EDGE_CURVE      = 1 << 7,
    LUMP_CHECK_NON_MANIFOLD_VTX     = 1 << 8,
    LUMP_CHECK_BAD_VOLUME           = 1 << 9,
    LUMP_CHECK_BAD_BOUNDING_BOX     = 1 << 10,
    LUMP_CHECK_SHELL_ORIENT_MISMATCH= 1 << 11,
    LUMP_CHECK_BAD_FACE_ADJACENCY   = 1 << 12,
    LUMP_CHECK_NON_MANIFOLD_EDGE    = 1 << 13
};

class lump_check_result {
public:
    lump_check_result();
    ~lump_check_result();

    int get_status() const;
    bool is_ok() const;
    void set_status(int status);

    int shell_count() const;
    int bad_face_count() const;
    int bad_edge_count() const;
    void note_shell();
    void note_bad_face();
    void note_bad_edge();

    void add_insanity(insanity_data *data);
    insanity_list *get_insanity_list();

private:
    int          _status;
    int          _shell_count;
    int          _bad_face_count;
    int          _bad_edge_count;
    insanity_list _insanities;
};

outcome api_check_lump(
    LUMP           *lump,
    lump_check_result &result,
    AcisOptions    *ao = NULL
);

logical check_lump_shells_valid(
    LUMP   *lump,
    insanity_list *ilist
);

logical check_shell_faces_valid(
    SHELL  *shell,
    insanity_list *ilist
);

logical check_lump_containment(
    LUMP   *lump,
    insanity_list *ilist
);

logical check_edge_curves_valid(
    FACE   *face,
    insanity_list *ilist
);

logical check_coedge_sense(
    FACE   *face,
    insanity_list *ilist
);

logical check_wire_self_intersect(
    WIRE   *wire,
    insanity_list *ilist
);

logical check_lump_volume(
    LUMP   *lump,
    insanity_list *ilist
);

logical check_lump_bounding_box(
    LUMP   *lump,
    insanity_list *ilist
);

logical check_lump_shell_orientation(
    LUMP   *lump,
    insanity_list *ilist
);

logical check_lump_face_adjacency(
    LUMP   *lump,
    insanity_list *ilist
);

logical check_lump_edge_manifold(
    LUMP   *lump,
    insanity_list *ilist
);

int api_check_lump_status(
    LUMP *lump,
    int  *insanity_count = NULL
);

#endif
