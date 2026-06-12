#include "check_vertex.hxx"
#include "vertex.hxx"
#include "edge.hxx"
#include "face.hxx"
#include "coedge.hxx"
#include "loop.hxx"
#include "shell.hxx"
#include "lump.hxx"
#include "body.hxx"
#include "point.hxx"
#include "curve.hxx"
#include "surface.hxx"
#include "spa_intr_solid.hxx"
#include <cmath>
#include <cstring>

vertex_check_result::vertex_check_result()
    : _status(VTX_CHECK_OK),
      _edge_count(0),
      _bad_edge_count(0),
      _non_manifold_count(0) {
}

vertex_check_result::~vertex_check_result() {
}

int vertex_check_result::get_status() const {
    return _status;
}

bool vertex_check_result::is_ok() const {
    return _status == VTX_CHECK_OK;
}

void vertex_check_result::set_status(int status) {
    _status = status;
}

int vertex_check_result::edge_count() const {
    return _edge_count;
}

int vertex_check_result::bad_edge_count() const {
    return _bad_edge_count;
}

int vertex_check_result::non_manifold_count() const {
    return _non_manifold_count;
}

void vertex_check_result::note_edge() {
    ++_edge_count;
}

void vertex_check_result::note_bad_edge() {
    ++_bad_edge_count;
}

void vertex_check_result::note_non_manifold() {
    ++_non_manifold_count;
}

void vertex_check_result::add_insanity(insanity_data *data) {
    if (data) {
        _insanities.add(data);
    }
}

insanity_list *vertex_check_result::get_insanity_list() {
    return &_insanities;
}

outcome api_check_vertex_errors(
    VERTEX            *vertex,
    vertex_check_result &result,
    AcisOptions       *ao
) {
    outcome res = outcome::success;

    if (!vertex || vertex->identity() != VERTEX_TYPE) {
        return outcome(API_NULL_ARGUMENT, FAIL, 0);
    }

<<<<<<< HEAD
    check_vertex_point_valid(vertex, result.get_insanity_list());

    check_vertex_edges_valid(vertex, result.get_insanity_list());

    check_vertex_edge_curves(vertex, result.get_insanity_list());

    check_vertex_coincident(vertex, result.get_insanity_list());

    check_vertex_edge_sense(vertex, result.get_insanity_list());

    check_vertex_manifold(vertex, result.get_insanity_list());

    check_vertex_bounding_box(vertex, result.get_insanity_list());

    check_vertex_normal_consistency(vertex, result.get_insanity_list());

    check_vertex_tolerance(vertex, result.get_insanity_list());

    check_vertex_sharp_angle(vertex, result.get_insanity_list());

    EDGE *edge = vertex->edge();
    EDGE *first_edge = edge;
    if (edge) {
        do {
            result.note_edge();
            if (!edge->start() || !edge->end() || !edge->curfi()) {
                result.note_bad_edge();
            }
            edge = edge->next(vertex);
        } while (edge && edge != first_edge);
    }

    int status = VTX_CHECK_OK;
    insanity_data *entry = result.get_insanity_list()->first();
    while (entry) {
        const char *desc = entry->get_description();
        if (desc) {
            if (strstr(desc, "null") || strstr(desc, "Null")) {
                status |= VTX_CHECK_NULL_POINT;
            }
            if (strstr(desc, "no incident edges")) {
                status |= VTX_CHECK_NO_EDGES;
            }
            if (strstr(desc, "degenerate")) {
                status |= VTX_CHECK_DEGENERATE_EDGE;
            }
            if (strstr(desc, "curve")) {
                status |= VTX_CHECK_BAD_EDGE_CURVE;
            }
            if (strstr(desc, "sense")) {
                status |= VTX_CHECK_EDGE_SENSE_MISMATCH;
            }
            if (strstr(desc, "non-manifold")) {
                status |= VTX_CHECK_NON_MANIFOLD;
                result.note_non_manifold();
            }
            if (strstr(desc, "coincident") || strstr(desc, "Coincident")) {
                status |= VTX_CHECK_COINCIDENT_VERTICES;
            }
            if (strstr(desc, "not at curve")) {
                status |= VTX_CHECK_POINT_NOT_ON_CURVE;
            }
            if (strstr(desc, "bounding box")) {
                status |= VTX_CHECK_BAD_BOUNDING_BOX;
            }
            if (strstr(desc, "normal")) {
                status |= VTX_CHECK_BAD_NORMAL_CONSISTENCY;
            }
            if (strstr(desc, "tolerance")) {
                status |= VTX_CHECK_BAD_TOLERANCE;
            }
            if (strstr(desc, "sharp")) {
                status |= VTX_CHECK_SHARP_ANGLE;
            }
        }
        entry = entry->next();
    }
    result.set_status(status);
=======
    int status = VTX_CHECK_OK;
>>>>>>> 5b669d23a53d60b412b23abc0b2023797e15d235

    check_vertex_point_valid(vertex, result.get_insanity_list(), &status);
    check_vertex_edges_valid(vertex, result.get_insanity_list(), &status);
    check_vertex_edge_curves(vertex, result.get_insanity_list(), &status);
    check_vertex_coincident(vertex, result.get_insanity_list(), &status);
    check_vertex_edge_sense(vertex, result.get_insanity_list(), &status);
    check_vertex_manifold(vertex, result.get_insanity_list(), &status);
    check_vertex_bounding_box(vertex, result.get_insanity_list(), &status);
    check_vertex_normal_consistency(vertex, result.get_insanity_list(), &status);
    check_vertex_tolerance(vertex, result.get_insanity_list(), &status);
    check_vertex_sharp_angle(vertex, result.get_insanity_list(), &status);

    result.set_status(status);
    return res;
}

logical check_vertex_point_valid(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    POINT *pt = vertex->point();
    if (!pt) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex has null point geometry.");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_NULL_POINT;
        return FALSE;
    }

    SPAposition pos = pt->position();

    if (std::isnan(pos.x()) || std::isnan(pos.y()) || std::isnan(pos.z())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex point has NaN coordinates.");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_NULL_POINT;
        return FALSE;
    }

    if (std::isinf(pos.x()) || std::isinf(pos.y()) || std::isinf(pos.z())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex point has infinite coordinates.");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_NULL_POINT;
        return FALSE;
    }

    return TRUE;
}

logical check_vertex_edges_valid(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    logical valid = TRUE;

    int edge_count = 0;
    EDGE *edge = vertex->edge();
    EDGE *first_edge = edge;

    if (!edge) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Vertex has no incident edges.");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_NO_EDGES;
        return FALSE;
    }

    do {
        edge_count++;

        VERTEX *v_start = edge->start();
        VERTEX *v_end = edge->end();
        if (!v_start || !v_end) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Edge has null start or end vertex.");
            ilist->add(id);
            if (status) *status |= VTX_CHECK_DEGENERATE_EDGE;
            valid = FALSE;
        } else {
            POINT *pt_start = v_start->point();
            POINT *pt_end = v_end->point();
            if (!pt_start || !pt_end) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Edge vertex has null point.");
                ilist->add(id);
                if (status) *status |= VTX_CHECK_DEGENERATE_EDGE;
                valid = FALSE;
            } else {
                SPAposition start_pos = pt_start->position();
                SPAposition end_pos = pt_end->position();
                double len = (end_pos - start_pos).length();

                if (len < SPAresabs) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(WARNING);
                    id->set_description("Edge is degenerate (zero length).");
                    ilist->add(id);
                    if (status) *status |= VTX_CHECK_DEGENERATE_EDGE;
                    valid = FALSE;
                }
            }
        }

        edge = edge->next(vertex);
    } while (edge && edge != first_edge);

    return valid;
}

logical check_vertex_edge_curves(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    logical valid = TRUE;
    POINT *v_pt = vertex->point();
    if (!v_pt) {
        return TRUE;
    }

    SPAposition vtx_pos = v_pt->position();
    EDGE *edge = vertex->edge();
    EDGE *first_edge = edge;

    if (!edge) {
        return TRUE;
    }

    do {
        CURVE *curve = edge->curfi();
        if (!curve) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Edge has null curve geometry.");
            ilist->add(id);
            if (status) *status |= VTX_CHECK_BAD_EDGE_CURVE;
            valid = FALSE;
        } else {
            double start_param = edge->start_param();
            double end_param = edge->end_param();

            double start_dist = (curve->eval_position(start_param) - vtx_pos).length();
            double end_dist = (curve->eval_position(end_param) - vtx_pos).length();

            if (edge->start() == vertex) {
                if (start_dist > SPAresabs) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(ERROR_TYPE);
                    id->set_description("Vertex not at curve start parameter position.");
                    ilist->add(id);
                    if (status) *status |= VTX_CHECK_BAD_EDGE_CURVE;
                    valid = FALSE;
                }
            } else if (edge->end() == vertex) {
                if (end_dist > SPAresabs) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(ERROR_TYPE);
                    id->set_description("Vertex not at curve end parameter position.");
                    ilist->add(id);
                    if (status) *status |= VTX_CHECK_BAD_EDGE_CURVE;
                    valid = FALSE;
                }
            }
        }

        edge = edge->next(vertex);
    } while (edge && edge != first_edge);

    return valid;
}

logical check_vertex_coincident(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    POINT *pt = vertex->point();
    if (!pt) {
        return TRUE;
    }

    SPAposition pos = pt->position();
    logical valid = TRUE;

    EDGE *edge = vertex->edge();
    EDGE *first_edge = edge;

    if (!edge) {
        return TRUE;
    }

    do {
        VERTEX *other = NULL;
        if (edge->start() == vertex) {
            other = edge->end();
        } else {
            other = edge->start();
        }

        if (other && other != vertex) {
            POINT *other_pt = other->point();
            if (other_pt) {
                SPAposition other_pos = other_pt->position();
                double dist = (pos - other_pos).length();

                if (dist < SPAresabs && dist > 0) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(WARNING);
                    id->set_description("Coincident vertices detected at edge endpoint.");
                    ilist->add(id);
                    if (status) *status |= VTX_CHECK_COINCIDENT_VERTICES;
                    valid = FALSE;
                }
            }
        }

        edge = edge->next(vertex);
    } while (edge && edge != first_edge);

    return valid;
}

logical check_vertex_edge_sense(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    logical valid = TRUE;
    EDGE *edge = vertex->edge();
    EDGE *first_edge = edge;

    if (!edge) {
        return TRUE;
    }

    do {
        COEDGE *coedge = edge->coedge();
        while (coedge) {
            COEDGE *partner = coedge->partner();
            if (partner) {
                if (partner->sense() == coedge->sense()) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(WARNING);
                    id->set_description(
                        "Coedge and partner have same sense at vertex."
                    );
                    ilist->add(id);
                    if (status) *status |= VTX_CHECK_EDGE_SENSE_MISMATCH;
                    valid = FALSE;
                }
            }
            coedge = coedge->next();
            if (coedge == edge->coedge()) break;
        }

        edge = edge->next(vertex);
    } while (edge && edge != first_edge);

    return valid;
}

logical check_vertex_manifold(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    int face_count = 0;
    EDGE *edge = vertex->edge();
    EDGE *first_edge = edge;

    if (!edge) {
        return TRUE;
    }

    do {
        COEDGE *coedge = edge->coedge();
        COEDGE *first_coedge = coedge;
        if (coedge) {
            do {
                FACE *face = coedge->loop() ? coedge->loop()->face() : NULL;
                if (face) {
                    face_count++;
                }
                coedge = coedge->next();
            } while (coedge && coedge != first_coedge);
        }

        edge = edge->next(vertex);
    } while (edge && edge != first_edge);

    if (face_count % 2 != 0 && face_count > 0) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Vertex may be non-manifold (odd number of faces).");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_NON_MANIFOLD;
        return FALSE;
    }

    return TRUE;
}

logical check_vertex_bounding_box(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    if (!vertex) {
        return FALSE;
    }

    POINT *pt = vertex->point();
    if (!pt) {
        return TRUE;
    }

    SPAposition pos = pt->position();

    if (std::isnan(pos.x()) || std::isnan(pos.y()) || std::isnan(pos.z())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex position contains NaN in bounding box.");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_BAD_BOUNDING_BOX;
        return FALSE;
    }

    if (std::isinf(pos.x()) || std::isinf(pos.y()) || std::isinf(pos.z())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex position contains Inf in bounding box.");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_BAD_BOUNDING_BOX;
        return FALSE;
    }

    return TRUE;
}

logical check_vertex_normal_consistency(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    if (!vertex) {
        return FALSE;
    }

    logical valid = TRUE;
    int normal_count = 0;
    int inconsistent_count = 0;

    EDGE *edge = vertex->edge();
    EDGE *first_edge = edge;

    if (!edge) {
        return TRUE;
    }

    do {
        COEDGE *coedge = edge->coedge();
        COEDGE *first_coedge = coedge;

        if (coedge) {
            do {
                FACE *face = coedge->loop() ? coedge->loop()->face() : NULL;
                if (face && face->surfi()) {
                    normal_count++;
                }
                coedge = coedge->next();
            } while (coedge && coedge != first_coedge);
        }

        edge = edge->next(vertex);
    } while (edge && edge != first_edge);

    if (normal_count > 1) {
        edge = vertex->edge();
        first_edge = edge;

        SPAvector first_normal(0, 0, 0);
        bool first_found = false;

        do {
            COEDGE *coedge = edge->coedge();
            COEDGE *first_coedge = coedge;

            if (coedge) {
                do {
                    FACE *face = coedge->loop() ? coedge->loop()->face() : NULL;
                    if (face && face->surfi()) {
                        SURFACE *surf = face->surfi();
                        SPApar_box pb = face->param_range();
                        double u_mid = (pb.interval(0).low() + pb.interval(0).high()) / 2.0;
                        double v_mid = (pb.interval(1).low() + pb.interval(1).high()) / 2.0;
                        SPApar_pos pos(u_mid, v_mid);

                        SPAvector du, dv;
                        try {
                            surf->eval_derivs(pos, du, dv);
                            SPAvector normal = du * dv;
                            double len = normal.length();
                            if (len > SPAresabs) {
                                normal = normal / len;
                                if (!first_found) {
                                    first_normal = normal;
                                    first_found = true;
                                } else {
                                    double dot = first_normal | normal;
                                    if (dot < 0.9) {
                                        insanity_data *id = new insanity_data();
                                        id->set_insanity_type(WARNING);
                                        id->set_description(
                                            "Vertex normal inconsistency detected."
                                        );
                                        ilist->add(id);
                                        if (status) *status |= VTX_CHECK_BAD_NORMAL_CONSISTENCY;
                                        valid = FALSE;
                                    }
                                }
                            }
                        } catch (...) {
                            // Skip on exception
                        }
                    }
                    coedge = coedge->next();
                } while (coedge && coedge != first_coedge);
            }

            edge = edge->next(vertex);
        } while (edge && edge != first_edge);
    }

    return valid;
}

logical check_vertex_tolerance(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    if (!vertex) {
        return FALSE;
    }

    logical valid = TRUE;

    double tolerance = vertex->tolerance();

    if (tolerance < 0) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex tolerance is negative.");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_BAD_TOLERANCE;
        valid = FALSE;
    }

    if (tolerance > SPAresabs * 10000) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Vertex tolerance is unusually large.");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_BAD_TOLERANCE;
    }

    if (std::isnan(tolerance) || std::isinf(tolerance)) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex tolerance is NaN or Inf.");
        ilist->add(id);
        if (status) *status |= VTX_CHECK_BAD_TOLERANCE;
        valid = FALSE;
    }

    return valid;
}

logical check_vertex_sharp_angle(
    VERTEX        *vertex,
    insanity_list *ilist,
    int           *status
) {
    if (!vertex) {
        return FALSE;
    }

    logical valid = TRUE;

    EDGE *edge = vertex->edge();
    EDGE *first_edge = edge;

    if (!edge) {
        return TRUE;
    }

    int edge_count = 0;
    EDGE *e = edge;
    do {
        edge_count++;
        e = e->next(vertex);
    } while (e && e != first_edge);

    if (edge_count < 2) {
        return TRUE;
    }

    // Collect edges into an array for pairwise angle comparison
    EDGE **edges = new EDGE*[edge_count];
    int idx = 0;
    e = edge;
    do {
        edges[idx++] = e;
        e = e->next(vertex);
    } while (e && e != first_edge);

    for (int i = 0; i < edge_count && valid; i++) {
        for (int j = i + 1; j < edge_count && valid; j++) {
            CURVE *c1 = edges[i]->curfi();
            CURVE *c2 = edges[j]->curfi();
            if (!c1 || !c2) continue;

            SPAvector tan1, tan2;
            try {
                if (edges[i]->start() == vertex) {
                    tan1 = c1->eval_deriv(edges[i]->start_param());
                } else {
                    tan1 = c1->eval_deriv(edges[i]->end_param());
                    tan1 = tan1 * (-1.0);
                }

                if (edges[j]->start() == vertex) {
                    tan2 = c2->eval_deriv(edges[j]->start_param());
                } else {
                    tan2 = c2->eval_deriv(edges[j]->end_param());
                    tan2 = tan2 * (-1.0);
                }

                double len1 = tan1.length();
                double len2 = tan2.length();
                if (len1 > SPAresabs && len2 > SPAresabs) {
                    double cos_angle = (tan1 | tan2) / (len1 * len2);
                    // Clamp to [-1, 1] to avoid numerical issues
                    if (cos_angle > 1.0) cos_angle = 1.0;
                    if (cos_angle < -1.0) cos_angle = -1.0;
                    double angle = acos(cos_angle);

                    // Check for very sharp angle (< ~10 degrees)
                    if (angle < 0.174) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(WARNING);
                        id->set_description(
                            "Sharp angle detected between edges at vertex."
                        );
                        ilist->add(id);
                        if (status) *status |= VTX_CHECK_SHARP_ANGLE;
                        valid = FALSE;
                    }
                }
            } catch (...) {
                // Skip on exception
            }
        }
    }

    delete[] edges;

    return valid;
}

int api_check_vertex(
    VERTEX *vertex,
    int    *insanity_count
) {
    if (!vertex || vertex->identity() != VERTEX_TYPE) {
        return VTX_CHECK_NULL_POINT;
    }

    insanity_list ilist;
    int count = 0;
    int status = VTX_CHECK_OK;

    if (check_vertex_point_valid(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_vertex_edges_valid(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_vertex_edge_curves(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_vertex_edge_sense(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_vertex_coincident(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_vertex_manifold(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_vertex_bounding_box(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_vertex_normal_consistency(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_vertex_tolerance(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_vertex_sharp_angle(vertex, &ilist, &status) == FALSE) {
        count++;
    }

    if (insanity_count) {
        *insanity_count = count;
    }

<<<<<<< HEAD
    int status = VTX_CHECK_OK;
    insanity_data *entry = ilist.first();
    while (entry) {
        const char *desc = entry->get_description();
        if (desc) {
            if (strstr(desc, "null") || strstr(desc, "Null")) {
                status |= VTX_CHECK_NULL_POINT;
            }
            if (strstr(desc, "no incident edges")) {
                status |= VTX_CHECK_NO_EDGES;
            }
            if (strstr(desc, "degenerate")) {
                status |= VTX_CHECK_DEGENERATE_EDGE;
            }
            if (strstr(desc, "curve")) {
                status |= VTX_CHECK_BAD_EDGE_CURVE;
            }
            if (strstr(desc, "sense")) {
                status |= VTX_CHECK_EDGE_SENSE_MISMATCH;
            }
            if (strstr(desc, "non-manifold")) {
                status |= VTX_CHECK_NON_MANIFOLD;
            }
            if (strstr(desc, "coincident") || strstr(desc, "Coincident")) {
                status |= VTX_CHECK_COINCIDENT_VERTICES;
            }
            if (strstr(desc, "not at curve")) {
                status |= VTX_CHECK_POINT_NOT_ON_CURVE;
            }
            if (strstr(desc, "bounding box")) {
                status |= VTX_CHECK_BAD_BOUNDING_BOX;
            }
            if (strstr(desc, "normal")) {
                status |= VTX_CHECK_BAD_NORMAL_CONSISTENCY;
            }
            if (strstr(desc, "tolerance")) {
                status |= VTX_CHECK_BAD_TOLERANCE;
            }
            if (strstr(desc, "angle") || strstr(desc, "sharp")) {
                status |= VTX_CHECK_SHARP_ANGLE;
            }
        }

        entry = entry->next();
    }

=======
>>>>>>> 5b669d23a53d60b412b23abc0b2023797e15d235
    return status;
}
