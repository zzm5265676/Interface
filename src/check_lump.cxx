#include "check_lump.hxx"
#include "face.hxx"
#include "edge.hxx"
#include "vertex.hxx"
#include "shell.hxx"
#include "lump.hxx"
#include "body.hxx"
#include "coedge.hxx"
#include "loop.hxx"
#include "wire.hxx"
#include "point.hxx"
#include "curve.hxx"
#include "surface.hxx"
#include "spa_intr_solid.hxx"
#include "intrapi.hxx"
#include "boolapi.hxx"
#include <cmath>
#include <cstring>
#include <vector>

lump_check_result::lump_check_result()
    : _status(LUMP_CHECK_OK),
      _shell_count(0),
      _bad_face_count(0),
      _bad_edge_count(0) {
}

lump_check_result::~lump_check_result() {
}

int lump_check_result::get_status() const {
    return _status;
}

bool lump_check_result::is_ok() const {
    return _status == LUMP_CHECK_OK;
}

void lump_check_result::set_status(int status) {
    _status = status;
}

int lump_check_result::shell_count() const {
    return _shell_count;
}

int lump_check_result::bad_face_count() const {
    return _bad_face_count;
}

int lump_check_result::bad_edge_count() const {
    return _bad_edge_count;
}

void lump_check_result::note_shell() {
    ++_shell_count;
}

void lump_check_result::note_bad_face() {
    ++_bad_face_count;
}

void lump_check_result::note_bad_edge() {
    ++_bad_edge_count;
}

void lump_check_result::add_insanity(insanity_data *data) {
    if (data) {
        _insanities.add(data);
    }
}

insanity_list *lump_check_result::get_insanity_list() {
    return &_insanities;
}

static logical is_inside_or_boundary(int containment) {
    return containment == point_inside || containment == point_boundary;
}

static logical is_outside_or_unknown(int containment) {
    return containment == point_outside || containment == point_unknown;
}

static logical check_lump_vertex_manifold(
    LUMP          *lump,
    insanity_list *ilist
) {
    logical valid = TRUE;
    std::vector<VERTEX*> seen_vertices;

    for (SHELL *shell = lump->shell(); shell; shell = shell->next()) {
        for (FACE *face = shell->face(); face; face = face->next()) {
            for (LOOP *loop = face->loop(); loop; loop = loop->next()) {
                COEDGE *coedge = loop->coedge();
                if (!coedge) {
                    continue;
                }

                COEDGE *first_coedge = coedge;
                do {
                    EDGE *edge = coedge->edge();
                    if (edge) {
                        VERTEX *vertices[2] = {edge->start(), edge->end()};
                        for (int i = 0; i < 2; ++i) {
                            VERTEX *vertex = vertices[i];
                            if (!vertex) {
                                continue;
                            }

                            bool already_seen = false;
                            for (size_t j = 0; j < seen_vertices.size(); ++j) {
                                if (seen_vertices[j] == vertex) {
                                    already_seen = true;
                                    break;
                                }
                            }
                            if (already_seen) {
                                continue;
                            }
                            seen_vertices.push_back(vertex);

                            int face_count = 0;
                            EDGE *incident = vertex->edge();
                            EDGE *first_incident = incident;
                            if (incident) {
                                do {
                                    COEDGE *incident_coedge = incident->coedge();
                                    COEDGE *first_incident_coedge = incident_coedge;
                                    if (incident_coedge) {
                                        do {
                                            FACE *incident_face =
                                                incident_coedge->loop() ?
                                                incident_coedge->loop()->face() : NULL;
                                            if (incident_face) {
                                                ++face_count;
                                            }
                                            incident_coedge = incident_coedge->next();
                                        } while (incident_coedge &&
                                                 incident_coedge != first_incident_coedge);
                                    }
                                    incident = incident->next(vertex);
                                } while (incident && incident != first_incident);
                            }

                            if (face_count > 0 && (face_count % 2) != 0) {
                                insanity_data *id = new insanity_data();
                                id->set_insanity_type(WARNING);
                                id->set_description(
                                    "Vertex may be non-manifold within lump."
                                );
                                ilist->add(id);
                                valid = FALSE;
                            }
                        }
                    }

                    coedge = coedge->next();
                } while (coedge && coedge != first_coedge);
            }
        }
    }

    return valid;
}

outcome api_check_lump(
    LUMP           *lump,
    lump_check_result &result,
    AcisOptions    *ao
) {
    outcome res = outcome::success;

    if (!lump || lump->identity() != LUMP_TYPE) {
        return outcome(API_NULL_ARGUMENT, FAIL, 0);
    }

    int status = LUMP_CHECK_OK;

    if (check_lump_shells_valid(lump, result.get_insanity_list()) == FALSE) {
        if (lump->shell()) {
            status |= LUMP_CHECK_EMPTY_SHELL;
        } else {
            status |= LUMP_CHECK_NO_SHELL;
        }
    }

    SHELL *shell = lump->shell();
    while (shell) {
        result.note_shell();

        if (check_shell_faces_valid(shell, result.get_insanity_list()) == FALSE) {
            status |= LUMP_CHECK_DEGENERATE_FACE;
            result.note_bad_face();
        }

        FACE *face = shell->face();
        while (face) {
            if (check_edge_curves_valid(face, result.get_insanity_list()) == FALSE) {
                result.note_bad_edge();
                status |= LUMP_CHECK_NULL_EDGE_CURVE;
            }

            if (check_coedge_sense(face, result.get_insanity_list()) == FALSE) {
                status |= LUMP_CHECK_BAD_COEDGE_SENSE;
            }

            LOOP *loop = face->loop();
            while (loop) {
                WIRE *wire = loop->wire();
                while (wire) {
                    if (check_wire_self_intersect(wire, result.get_insanity_list()) == FALSE) {
                        status |= LUMP_CHECK_SHELL_SELF_INT;
                    }
                    wire = wire->next();
                }
                loop = loop->next();
            }

            face = face->next();
        }

        shell = shell->next();
    }

    if (check_lump_containment(lump, result.get_insanity_list()) == FALSE) {
        status |= LUMP_CHECK_BAD_CONTAINMENT;
    }

    if (check_lump_volume(lump, result.get_insanity_list()) == FALSE) {
        status |= LUMP_CHECK_BAD_VOLUME;
    }

    if (check_lump_bounding_box(lump, result.get_insanity_list()) == FALSE) {
        status |= LUMP_CHECK_BAD_BOUNDING_BOX;
    }

    if (check_lump_shell_orientation(lump, result.get_insanity_list()) == FALSE) {
        status |= LUMP_CHECK_SHELL_ORIENT_MISMATCH;
    }

    if (check_lump_face_adjacency(lump, result.get_insanity_list()) == FALSE) {
        status |= LUMP_CHECK_BAD_FACE_ADJACENCY;
    }

    if (check_lump_edge_manifold(lump, result.get_insanity_list()) == FALSE) {
        status |= LUMP_CHECK_NON_MANIFOLD_EDGE;
    }

    if (check_lump_vertex_manifold(lump, result.get_insanity_list()) == FALSE) {
        status |= LUMP_CHECK_NON_MANIFOLD_VTX;
    }

    result.set_status(status);
    return res;
}

logical check_lump_shells_valid(
    LUMP   *lump,
    insanity_list *ilist
) {
    SHELL *shell = lump->shell();

    if (!shell) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Lump has no shells.");
        ilist->add(id);
        return FALSE;
    }

    logical valid = TRUE;
    shell = lump->shell();
    while (shell) {
        if (!shell->face() && !shell->wire()) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(WARNING);
            id->set_description("Shell contains no faces and no wires.");
            ilist->add(id);
            valid = FALSE;
        }
        shell = shell->next();
    }

    return valid;
}

logical check_shell_faces_valid(
    SHELL  *shell,
    insanity_list *ilist
) {
    logical valid = TRUE;
    FACE *face = shell->face();

    while (face) {
        if (!face->surfi()) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Face has null surface.");
            ilist->add(id);
            valid = FALSE;
        }

        LOOP *loop = face->loop();
        while (loop) {
            COEDGE *coedge = loop->coedge();
            if (!coedge) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Loop has no coedge.");
                ilist->add(id);
                valid = FALSE;
            }
            loop = loop->next();
        }

        face = face->next();
    }

    return valid;
}

logical check_lump_containment(
    LUMP   *lump,
    insanity_list *ilist
) {
    int shell_count = 0;
    SHELL *first_shell = NULL;

    SHELL *s = lump->shell();
    while (s) {
        if (!first_shell) first_shell = s;
        shell_count++;
        s = s->next();
    }

    if (shell_count < 2) {
        return TRUE;
    }

    logical valid = TRUE;
    SHELL *outer = lump->shell();
    while (outer) {
        FACE *outer_face = outer->face();
        if (!outer_face) {
            outer = outer->next();
            continue;
        }

        SHELL *inner = outer->next();
        while (inner) {
            FACE *inner_face = inner->face();
            if (!inner_face) {
                inner = inner->next();
                continue;
            }

            SPAposition outer_pt = outer_face->equation_position(
                (outer_face->start_u() + outer_face->end_u()) / 2.0,
                (outer_face->start_v() + outer_face->end_v()) / 2.0
            );

            SPAposition inner_pt = inner_face->equation_position(
                (inner_face->start_u() + inner_face->end_u()) / 2.0,
                (inner_face->start_v() + inner_face->end_v()) / 2.0
            );

            int outer_containment = point_in_shell(outer, outer_pt);
            int inner_containment = point_in_shell(inner, inner_pt);
            int inner_in_outer = point_in_shell(outer, inner_pt);
            int outer_in_inner = point_in_shell(inner, outer_pt);

            if (is_outside_or_unknown(outer_containment) ||
                is_outside_or_unknown(inner_containment) ||
                !is_inside_or_boundary(inner_in_outer) ||
                !is_outside_or_unknown(outer_in_inner)) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description(
                    "Improper shell containment in lump: "
                    "shells have inconsistent containment."
                );
                ilist->add(id);
                valid = FALSE;
            }

            inner = inner->next();
        }
        outer = outer->next();
    }

    return valid;
}

logical check_edge_curves_valid(
    FACE   *face,
    insanity_list *ilist
) {
    logical valid = TRUE;
    LOOP *loop = face->loop();

    while (loop) {
        COEDGE *coedge = loop->coedge();
        if (!coedge) {
            loop = loop->next();
            continue;
        }

        COEDGE *first_coedge = coedge;
        do {
            EDGE *edge = coedge->edge();
            if (edge) {
                CURVE *curve = edge->curfi();
                if (!curve) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(ERROR_TYPE);
                    id->set_description("Edge has null curve geometry.");
                    ilist->add(id);
                    valid = FALSE;
                }

                VERTEX *v_start = edge->start();
                VERTEX *v_end = edge->end();
                if (!v_start || !v_end) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(ERROR_TYPE);
                    id->set_description("Edge has null start or end vertex.");
                    ilist->add(id);
                    valid = FALSE;
                } else {
                    if (!v_start->point()) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(ERROR_TYPE);
                        id->set_description("Start vertex has null point.");
                        ilist->add(id);
                        valid = FALSE;
                    }
                    if (!v_end->point()) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(ERROR_TYPE);
                        id->set_description("End vertex has null point.");
                        ilist->add(id);
                        valid = FALSE;
                    }
                }
            } else {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Coedge has null edge.");
                ilist->add(id);
                valid = FALSE;
            }

            coedge = coedge->next();
        } while (coedge != first_coedge);

        loop = loop->next();
    }

    return valid;
}

logical check_coedge_sense(
    FACE   *face,
    insanity_list *ilist
) {
    logical valid = TRUE;
    LOOP *loop = face->loop();

    while (loop) {
        COEDGE *coedge = loop->coedge();
        if (!coedge) {
            loop = loop->next();
            continue;
        }

        COEDGE *first_coedge = coedge;
        do {
            COEDGE *partner = coedge->partner();
            if (partner) {
                if (partner->sense() == coedge->sense()) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(WARNING);
                    id->set_description(
                        "Coedge and its partner have the same sense."
                    );
                    ilist->add(id);
                    valid = FALSE;
                }
            }

            coedge = coedge->next();
        } while (coedge != first_coedge);

        loop = loop->next();
    }

    return valid;
}

logical check_wire_self_intersect(
    WIRE   *wire,
    insanity_list *ilist
) {
    logical valid = TRUE;
    COEDGE *coedge = wire->coedge();
    if (!coedge) {
        return TRUE;
    }

    COEDGE *first_coedge = coedge;
    do {
        COEDGE *other = coedge->next();
        if (!other) break;

        COEDGE *other_first = other;
        do {
            EDGE *e1 = coedge->edge();
            EDGE *e2 = other->edge();

            if (e1 && e2 && e1 != e2) {
                SPAintervalRange range1 = e1->param_range();
                SPAintervalRange range2 = e2->param_range();

                int num_int;
                double *par1 = NULL;
                double *par2 = NULL;
                SPAposition *pts = NULL;

                num_int = find_intersections(
                    *e1, range1, *e2, range2,
                    par1, par2, pts
                );

                for (int i = 0; i < num_int; i++) {
                    logical at_endpoint = FALSE;

                    if (fabs(par1[i] - range1.low()) < SPAresabs ||
                        fabs(par1[i] - range1.high()) < SPAresabs) {
                        if (fabs(par2[i] - range2.low()) < SPAresabs ||
                            fabs(par2[i] - range2.high()) < SPAresabs) {
                            at_endpoint = TRUE;
                        }
                    }

                    if (!at_endpoint) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(ERROR_TYPE);
                        id->set_description(
                            "Wire self-intersection detected."
                        );
                        ilist->add(id);
                        valid = FALSE;
                    }
                }

                ACIS_DELETE[] par1;
                ACIS_DELETE[] par2;
                ACIS_DELETE[] pts;
            }

            other = other->next();
            if (other == first_coedge) break;
        } while (other != other_first);

        coedge = coedge->next();
    } while (coedge != first_coedge);

    return valid;
}

logical check_lump_volume(
    LUMP   *lump,
    insanity_list *ilist
) {
    if (!lump) {
        return FALSE;
    }

    logical valid = TRUE;

    BODY *body = lump->body();
    if (!body) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Lump has no owning body.");
        ilist->add(id);
        valid = FALSE;
    }

    SHELL *shell = lump->shell();
    if (!shell) {
        return TRUE;
    }

    int shell_count = 0;
    while (shell) {
        shell_count++;
        shell = shell->next();
    }

    if (shell_count == 0) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Lump has zero shells.");
        ilist->add(id);
        valid = FALSE;
    }

    return valid;
}

logical check_lump_bounding_box(
    LUMP   *lump,
    insanity_list *ilist
) {
    if (!lump) {
        return FALSE;
    }

    logical valid = TRUE;

    SHELL *shell = lump->shell();
    while (shell) {
        FACE *face = shell->face();
        while (face) {
            LOOP *loop = face->loop();
            while (loop) {
                COEDGE *coedge = loop->coedge();
                if (coedge) {
                    COEDGE *first_coedge = coedge;
                    do {
                        EDGE *edge = coedge->edge();
                        if (edge) {
                            VERTEX *v_start = edge->start();
                            VERTEX *v_end = edge->end();

                            if (v_start && v_start->point()) {
                                SPAposition pos = v_start->point()->position();
                                if (std::isnan(pos.x()) || std::isnan(pos.y()) ||
                                    std::isnan(pos.z())) {
                                    insanity_data *id = new insanity_data();
                                    id->set_insanity_type(ERROR_TYPE);
                                    id->set_description(
                                        "Lump contains vertex with NaN in bounding box."
                                    );
                                    ilist->add(id);
                                    valid = FALSE;
                                }
                            }

                            if (v_end && v_end->point()) {
                                SPAposition pos = v_end->point()->position();
                                if (std::isnan(pos.x()) || std::isnan(pos.y()) ||
                                    std::isnan(pos.z())) {
                                    insanity_data *id = new insanity_data();
                                    id->set_insanity_type(ERROR_TYPE);
                                    id->set_description(
                                        "Lump contains vertex with NaN in bounding box."
                                    );
                                    ilist->add(id);
                                    valid = FALSE;
                                }
                            }
                        }
                        coedge = coedge->next();
                    } while (coedge && coedge != first_coedge);
                }
                loop = loop->next();
            }
            face = face->next();
        }
        shell = shell->next();
    }

    return valid;
}

logical check_lump_shell_orientation(
    LUMP   *lump,
    insanity_list *ilist
) {
    if (!lump) {
        return FALSE;
    }

    logical valid = TRUE;

    SHELL *shell = lump->shell();
    int shell_idx = 0;

    while (shell) {
        FACE *face = shell->face();
        if (face) {
            LOOP *loop = face->loop();
            if (loop) {
                COEDGE *coedge = loop->coedge();
                if (coedge) {
                    int forward_count = 0;
                    int reverse_count = 0;

                    COEDGE *first_coedge = coedge;
                    do {
                        if (coedge->sense() == 0) {
                            forward_count++;
                        } else {
                            reverse_count++;
                        }
                        coedge = coedge->next();
                    } while (coedge && coedge != first_coedge);

                    if (forward_count > 0 && reverse_count > 0) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(WARNING);
                        id->set_description(
                            "Shell has mixed coedge orientations in a loop."
                        );
                        ilist->add(id);
                        valid = FALSE;
                    }
                }
            }
        }

        shell_idx++;
        shell = shell->next();
    }

    return valid;
}

logical check_lump_face_adjacency(
    LUMP   *lump,
    insanity_list *ilist
) {
    if (!lump) {
        return FALSE;
    }

    logical valid = TRUE;

    SHELL *shell = lump->shell();
    while (shell) {
        FACE *face = shell->face();
        while (face) {
            LOOP *loop = face->loop();
            while (loop) {
                COEDGE *coedge = loop->coedge();
                if (coedge) {
                    COEDGE *first_coedge = coedge;
                    do {
                        COEDGE *partner = coedge->partner();
                        if (!partner) {
                            insanity_data *id = new insanity_data();
                            id->set_insanity_type(WARNING);
                            id->set_description(
                                "Face has coedge without partner (free edge)."
                            );
                            ilist->add(id);
                            valid = FALSE;
                        }
                        coedge = coedge->next();
                    } while (coedge && coedge != first_coedge);
                }
                loop = loop->next();
            }
            face = face->next();
        }
        shell = shell->next();
    }

    return valid;
}

logical check_lump_edge_manifold(
    LUMP   *lump,
    insanity_list *ilist
) {
    if (!lump) {
        return FALSE;
    }

    logical valid = TRUE;

    SHELL *shell = lump->shell();
    while (shell) {
        FACE *face = shell->face();
        while (face) {
            LOOP *loop = face->loop();
            while (loop) {
                COEDGE *coedge = loop->coedge();
                if (coedge) {
                    COEDGE *first_coedge = coedge;
                    do {
                        EDGE *edge = coedge->edge();
                        if (edge) {
                            int coedge_count = 0;
                            COEDGE *e_coedge = edge->coedge();
                            if (e_coedge) {
                                COEDGE *e_first = e_coedge;
                                do {
                                    coedge_count++;
                                    e_coedge = e_coedge->next();
                                } while (e_coedge && e_coedge != e_first);
                            }

                            if (coedge_count > 0 && coedge_count % 2 != 0) {
                                insanity_data *id = new insanity_data();
                                id->set_insanity_type(WARNING);
                                id->set_description(
                                    "Edge is non-manifold (odd coedge count)."
                                );
                                ilist->add(id);
                                valid = FALSE;
                            }
                        }
                        coedge = coedge->next();
                    } while (coedge && coedge != first_coedge);
                }
                loop = loop->next();
            }
            face = face->next();
        }
        shell = shell->next();
    }

    return valid;
}

int api_check_lump_status(
    LUMP *lump,
    int  *insanity_count
) {
    if (!lump || lump->identity() != LUMP_TYPE) {
        return LUMP_CHECK_NO_SHELL;
    }

    insanity_list ilist;
    int count = 0;

    if (check_lump_shells_valid(lump, &ilist) == FALSE) {
        count++;
    }

    if (check_lump_containment(lump, &ilist) == FALSE) {
        count++;
    }

    if (check_lump_volume(lump, &ilist) == FALSE) {
        count++;
    }

    if (check_lump_bounding_box(lump, &ilist) == FALSE) {
        count++;
    }

    if (check_lump_shell_orientation(lump, &ilist) == FALSE) {
        count++;
    }

    if (check_lump_face_adjacency(lump, &ilist) == FALSE) {
        count++;
    }

    if (check_lump_edge_manifold(lump, &ilist) == FALSE) {
        count++;
    }

    if (check_lump_vertex_manifold(lump, &ilist) == FALSE) {
        count++;
    }

    SHELL *shell = lump->shell();
    while (shell) {
        if (check_shell_faces_valid(shell, &ilist) == FALSE) {
            count++;
        }

        FACE *face = shell->face();
        while (face) {
            if (check_edge_curves_valid(face, &ilist) == FALSE) {
                count++;
            }
            if (check_coedge_sense(face, &ilist) == FALSE) {
                count++;
            }
            face = face->next();
        }

        shell = shell->next();
    }

    if (insanity_count) {
        *insanity_count = count;
    }

    int status = LUMP_CHECK_OK;
    insanity_data *entry = ilist.first();
    while (entry) {
        const char *desc = entry->get_description();
        if (desc) {
            if (strstr(desc, "no shells") || strstr(desc, "zero shells")) {
                status |= LUMP_CHECK_NO_SHELL;
            }
            if (strstr(desc, "empty") ||
                strstr(desc, "no faces and no wires")) {
                status |= LUMP_CHECK_EMPTY_SHELL;
            }
            if (strstr(desc, "containment")) {
                status |= LUMP_CHECK_BAD_CONTAINMENT;
            }
            if (strstr(desc, "volume")) {
                status |= LUMP_CHECK_BAD_VOLUME;
            }
            if (strstr(desc, "bounding box")) {
                status |= LUMP_CHECK_BAD_BOUNDING_BOX;
            }
            if (strstr(desc, "orientation")) {
                status |= LUMP_CHECK_SHELL_ORIENT_MISMATCH;
            }
            if (strstr(desc, "adjacency") || strstr(desc, "free edge")) {
                status |= LUMP_CHECK_BAD_FACE_ADJACENCY;
            }
            if (strstr(desc, "non-manifold within lump")) {
                status |= LUMP_CHECK_NON_MANIFOLD_VTX;
            } else if (strstr(desc, "manifold")) {
                status |= LUMP_CHECK_NON_MANIFOLD_EDGE;
            }
            if (strstr(desc, "null curve") || strstr(desc, "null point")) {
                status |= LUMP_CHECK_NULL_EDGE_CURVE;
            }
            if (strstr(desc, "self-intersection")) {
                status |= LUMP_CHECK_SHELL_SELF_INT;
            }
            if (strstr(desc, "null surface") || strstr(desc, "no coedge")) {
                status |= LUMP_CHECK_DEGENERATE_FACE;
            }
            if (strstr(desc, "same sense")) {
                status |= LUMP_CHECK_BAD_COEDGE_SENSE;
            }
        }

        entry = entry->next();
    }

    return status;
}
