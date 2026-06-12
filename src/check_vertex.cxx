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
            }
            if (strstr(desc, "coincident")) {
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

    return res;
}

logical check_vertex_point_valid(
    VERTEX        *vertex,
    insanity_list *ilist
) {
    POINT *pt = vertex->point();
    if (!pt) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex has null point geometry.");
        ilist->add(id);
        return FALSE;
    }

    SPAposition pos = pt->position();

    if (std::isnan(pos.x()) || std::isnan(pos.y()) || std::isnan(pos.z())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex point has NaN coordinates.");
        ilist->add(id);
        return FALSE;
    }

    if (std::isinf(pos.x()) || std::isinf(pos.y()) || std::isinf(pos.z())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex point has infinite coordinates.");
        ilist->add(id);
        return FALSE;
    }

    return TRUE;
}

logical check_vertex_edges_valid(
    VERTEX        *vertex,
    insanity_list *ilist
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
            valid = FALSE;
        } else {
            POINT *pt_start = v_start->point();
            POINT *pt_end = v_end->point();
            if (!pt_start || !pt_end) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Edge vertex has null point.");
                ilist->add(id);
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
    insanity_list *ilist
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
                    valid = FALSE;
                }
            } else if (edge->end() == vertex) {
                if (end_dist > SPAresabs) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(ERROR_TYPE);
                    id->set_description("Vertex not at curve end parameter position.");
                    ilist->add(id);
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
    insanity_list *ilist
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
    insanity_list *ilist
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
    insanity_list *ilist
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
        return FALSE;
    }

    return TRUE;
}

logical check_vertex_bounding_box(
    VERTEX        *vertex,
    insanity_list *ilist
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
        return FALSE;
    }

    if (std::isinf(pos.x()) || std::isinf(pos.y()) || std::isinf(pos.z())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex position contains Inf in bounding box.");
        ilist->add(id);
        return FALSE;
    }

    return TRUE;
}

logical check_vertex_normal_consistency(
    VERTEX        *vertex,
    insanity_list *ilist
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

        SPAvector first_normal;
        bool first_found = false;

        do {
            COEDGE *coedge = edge->coedge();
            COEDGE *first_coedge = coedge;

            if (coedge) {
                do {
                    FACE *face = coedge->loop() ? coedge->loop()->face() : NULL;
                    if (face && face->surfi()) {
                        if (!first_found) {
                            first_found = true;
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
    insanity_list *ilist
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
        valid = FALSE;
    }

    if (tolerance > SPAresabs * 10000) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Vertex tolerance is unusually large.");
        ilist->add(id);
    }

    if (std::isnan(tolerance) || std::isinf(tolerance)) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Vertex tolerance is NaN or Inf.");
        ilist->add(id);
        valid = FALSE;
    }

    return valid;
}

logical check_vertex_sharp_angle(
    VERTEX        *vertex,
    insanity_list *ilist
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

    double** angles = new double*[edge_count];
    for (int i = 0; i < edge_count; i++) {
        angles[i] = new double[edge_count];
    }

    int idx1 = 0;
    e = edge;
    do {
        int idx2 = 0;
        EDGE *e2 = edge;
        do {
            if (idx1 != idx2) {
                angles[idx1][idx2] = 0.0;
            }
            idx2++;
            e2 = e2->next(vertex);
        } while (e2 && e2 != first_edge);

        idx1++;
        e = e->next(vertex);
    } while (e && e != first_edge);

    for (int i = 0; i < edge_count; i++) {
        delete[] angles[i];
    }
    delete[] angles;

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

    if (check_vertex_point_valid(vertex, &ilist) == FALSE) {
        count++;
    }

    if (check_vertex_edges_valid(vertex, &ilist) == FALSE) {
        count++;
    }

    if (check_vertex_edge_curves(vertex, &ilist) == FALSE) {
        count++;
    }

    if (check_vertex_edge_sense(vertex, &ilist) == FALSE) {
        count++;
    }

    if (check_vertex_coincident(vertex, &ilist) == FALSE) {
        count++;
    }

    if (check_vertex_manifold(vertex, &ilist) == FALSE) {
        count++;
    }

    if (check_vertex_bounding_box(vertex, &ilist) == FALSE) {
        count++;
    }

    if (check_vertex_normal_consistency(vertex, &ilist) == FALSE) {
        count++;
    }

    if (check_vertex_tolerance(vertex, &ilist) == FALSE) {
        count++;
    }

    if (check_vertex_sharp_angle(vertex, &ilist) == FALSE) {
        count++;
    }

    if (insanity_count) {
        *insanity_count = count;
    }

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
            if (strstr(desc, "Coincident")) {
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

    return status;
}
