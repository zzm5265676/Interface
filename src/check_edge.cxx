#include "check_edge.hxx"
#include "edge.hxx"
#include "vertex.hxx"
#include "point.hxx"
#include "curve.hxx"
#include "coedge.hxx"
#include "loop.hxx"
#include "face.hxx"
#include "SPAres.hxx"
#include <cmath>
#include <cstring>

edge_check_result::edge_check_result()
    : _status(EDGE_CHECK_OK),
      _eval_failure_count(0) {
}

edge_check_result::~edge_check_result() {
}

int edge_check_result::get_status() const {
    return _status;
}

bool edge_check_result::is_ok() const {
    return _status == EDGE_CHECK_OK;
}

void edge_check_result::set_status(int status) {
    _status = status;
}

int edge_check_result::eval_failure_count() const {
    return _eval_failure_count;
}

void edge_check_result::note_eval_failure() {
    ++_eval_failure_count;
}

void edge_check_result::add_insanity(insanity_data *data) {
    if (data) {
        _insanities.add(data);
    }
}

insanity_list *edge_check_result::get_insanity_list() {
    return &_insanities;
}

outcome api_check_edge_errors(
    EDGE              *edge,
    edge_check_result &result,
    AcisOptions       *ao
) {
    outcome res = outcome::success;

    if (!edge || edge->identity() != EDGE_TYPE) {
        return outcome(API_NULL_ARGUMENT, FAIL, 0);
    }

    int status = EDGE_CHECK_OK;
<<<<<<< HEAD
    insanity_data *entry = result.get_insanity_list()->first();
    while (entry) {
        const char *desc = entry->get_description();
        if (desc) {
            if (strstr(desc, "null")) {
                if (strstr(desc, "curve")) {
                    status |= EDGE_CHECK_NULL_CURVE;
                } else if (strstr(desc, "vertex")) {
                    status |= EDGE_CHECK_NULL_VERTEX;
                } else if (strstr(desc, "Edge")) {
                    status |= EDGE_CHECK_NULL_EDGE;
                }
            }
            if (strstr(desc, "degenerate")) {
                status |= EDGE_CHECK_DEGENERATE;
            }
            if (strstr(desc, "parameter")) {
                status |= EDGE_CHECK_BAD_PARAM_RANGE;
            }
            if (strstr(desc, "not at curve")) {
                status |= EDGE_CHECK_VERTEX_NOT_ON_CURVE;
            }
            if (strstr(desc, "closure") || strstr(desc, "closed")) {
                status |= EDGE_CHECK_BAD_CLOSURE;
            }
            if (strstr(desc, "sense")) {
                status |= EDGE_CHECK_COEDGE_SENSE_ERROR;
            }
            if (strstr(desc, "evaluation") || strstr(desc, "threw")) {
                status |= EDGE_CHECK_EVAL_FAILURE;
                result.note_eval_failure();
            }
            if (strstr(desc, "NaN") || strstr(desc, "Inf")) {
                status |= EDGE_CHECK_NAN_COORDINATES;
            }
            if (strstr(desc, "tolerance")) {
                status |= EDGE_CHECK_BAD_FIT_TOLERANCE;
            }
            if (strstr(desc, "length")) {
                status |= EDGE_CHECK_BAD_LENGTH;
            }
            if (strstr(desc, "G1")) {
                status |= EDGE_CHECK_NON_G1_CONTINUITY;
            }
            if (strstr(desc, "bounding box")) {
                status |= EDGE_CHECK_BAD_BOUNDING_BOX;
            }
            if (strstr(desc, "normalization") ||
                strstr(desc, "start parameter > end parameter")) {
                status |= EDGE_CHECK_BAD_PARAM_NORMALIZATION;
            }
        }
        entry = entry->next();
    }
    result.set_status(status);
=======
>>>>>>> 5b669d23a53d60b412b23abc0b2023797e15d235

    check_edge_null(edge, result.get_insanity_list(), &status);
    check_edge_curve(edge, result.get_insanity_list(), &status);
    check_edge_vertices(edge, result.get_insanity_list(), &status);
    check_edge_degenerate(edge, result.get_insanity_list(), &status);
    check_edge_parameter_range(edge, result.get_insanity_list(), &status);
    check_edge_vertex_on_curve(edge, result.get_insanity_list(), &status);
    check_edge_closure(edge, result.get_insanity_list(), &status);
    check_edge_coedge_sense(edge, result.get_insanity_list(), &status);
    check_edge_evaluation(edge, result.get_insanity_list(), &status);
    check_edge_fit_tolerance(edge, result.get_insanity_list(), &status);
    check_edge_length(edge, result.get_insanity_list(), &status);
    check_edge_g1_continuity(edge, result.get_insanity_list(), &status);
    check_edge_bounding_box(edge, result.get_insanity_list(), &status);
    check_edge_param_normalization(edge, result.get_insanity_list(), &status);

    result.set_status(status);
    return res;
}

logical check_edge_null(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge pointer is null.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_NULL_EDGE;
        return FALSE;
    }

    return TRUE;
}

logical check_edge_curve(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    CURVE *curve = edge->curfi();
    if (!curve) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge has null curve geometry.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_NULL_CURVE;
        return FALSE;
    }

    return TRUE;
}

logical check_edge_vertices(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    logical valid = TRUE;

    VERTEX *v_start = edge->start();
    VERTEX *v_end = edge->end();

    if (!v_start) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge has null start vertex.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_NULL_VERTEX;
        valid = FALSE;
    } else {
        POINT *pt = v_start->point();
        if (!pt) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Edge start vertex has null point.");
            ilist->add(id);
            if (status) *status |= EDGE_CHECK_NULL_VERTEX;
            valid = FALSE;
        } else {
            SPAposition pos = pt->position();
            if (std::isnan(pos.x()) || std::isnan(pos.y()) ||
                std::isnan(pos.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Edge start vertex has NaN position.");
                ilist->add(id);
                if (status) *status |= EDGE_CHECK_NAN_COORDINATES;
                valid = FALSE;
            }
            if (std::isinf(pos.x()) || std::isinf(pos.y()) ||
                std::isinf(pos.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Edge start vertex has Inf position.");
                ilist->add(id);
                if (status) *status |= EDGE_CHECK_NAN_COORDINATES;
                valid = FALSE;
            }
        }
    }

    if (!v_end) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge has null end vertex.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_NULL_VERTEX;
        valid = FALSE;
    } else {
        POINT *pt = v_end->point();
        if (!pt) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Edge end vertex has null point.");
            ilist->add(id);
            if (status) *status |= EDGE_CHECK_NULL_VERTEX;
            valid = FALSE;
        } else {
            SPAposition pos = pt->position();
            if (std::isnan(pos.x()) || std::isnan(pos.y()) ||
                std::isnan(pos.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Edge end vertex has NaN position.");
                ilist->add(id);
                if (status) *status |= EDGE_CHECK_NAN_COORDINATES;
                valid = FALSE;
            }
            if (std::isinf(pos.x()) || std::isinf(pos.y()) ||
                std::isinf(pos.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Edge end vertex has Inf position.");
                ilist->add(id);
                if (status) *status |= EDGE_CHECK_NAN_COORDINATES;
                valid = FALSE;
            }
        }
    }

    return valid;
}

logical check_edge_degenerate(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    VERTEX *v_start = edge->start();
    VERTEX *v_end = edge->end();

    if (!v_start || !v_end) {
        return TRUE;
    }

    POINT *pt_start = v_start->point();
    POINT *pt_end = v_end->point();

    if (!pt_start || !pt_end) {
        return TRUE;
    }

    SPAposition start_pos = pt_start->position();
    SPAposition end_pos = pt_end->position();
    double length = (end_pos - start_pos).length();

    if (length < SPAresabs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Edge is degenerate (zero length).");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_DEGENERATE;
        return FALSE;
    }

    return TRUE;
}

logical check_edge_parameter_range(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    SPAinterval range = edge->param_range();

    if (range.null()) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge parameter range is null.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_PARAM_RANGE;
        return FALSE;
    }

    if (range.high() - range.low() < SPAresabs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Edge parameter range is degenerate.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_PARAM_RANGE;
    }

    if (std::isnan(range.low()) || std::isnan(range.high())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge parameter range contains NaN.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_NAN_COORDINATES;
        return FALSE;
    }

    if (std::isinf(range.low()) || std::isinf(range.high())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge parameter range contains Inf.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_NAN_COORDINATES;
        return FALSE;
    }

    return TRUE;
}

logical check_edge_vertex_on_curve(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    CURVE *curve = edge->curfi();
    if (!curve) {
        return TRUE;
    }

    logical valid = TRUE;

    VERTEX *v_start = edge->start();
    VERTEX *v_end = edge->end();

    if (v_start && v_start->point()) {
        SPAposition start_pos = v_start->point()->position();
        double start_param = edge->start_param();
        SPAposition curve_start = curve->eval_position(start_param);

        if ((start_pos - curve_start).length() > SPAresabs) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description(
                "Edge start vertex not at curve start parameter position."
            );
            ilist->add(id);
            if (status) *status |= EDGE_CHECK_VERTEX_NOT_ON_CURVE;
            valid = FALSE;
        }
    }

    if (v_end && v_end->point()) {
        SPAposition end_pos = v_end->point()->position();
        double end_param = edge->end_param();
        SPAposition curve_end = curve->eval_position(end_param);

        if ((end_pos - curve_end).length() > SPAresabs) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description(
                "Edge end vertex not at curve end parameter position."
            );
            ilist->add(id);
            if (status) *status |= EDGE_CHECK_VERTEX_NOT_ON_CURVE;
            valid = FALSE;
        }
    }

    return valid;
}

logical check_edge_closure(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    if (!edge->closed()) {
        return TRUE;
    }

    CURVE *curve = edge->curfi();
    if (!curve) {
        return TRUE;
    }

    logical valid = TRUE;
    SPAinterval range = edge->param_range();

    SPAposition start_pos = curve->eval_position(range.low());
    SPAposition end_pos = curve->eval_position(range.high());

    if ((start_pos - end_pos).length() > SPAresabs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description(
            "Edge marked closed but start/end positions differ."
        );
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_CLOSURE;
        valid = FALSE;
    }

    VERTEX *v_start = edge->start();
    VERTEX *v_end = edge->end();

    if (v_start && v_end && v_start != v_end) {
        if (v_start->point() && v_end->point()) {
            SPAposition vtx_start = v_start->point()->position();
            SPAposition vtx_end = v_end->point()->position();

            if ((vtx_start - vtx_end).length() > SPAresabs) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description(
                    "Edge closed but start/end vertices differ."
                );
                ilist->add(id);
                if (status) *status |= EDGE_CHECK_BAD_CLOSURE;
                valid = FALSE;
            }
        }
    }

    return valid;
}

logical check_edge_coedge_sense(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    logical valid = TRUE;

    COEDGE *coedge = edge->coedge();
    if (!coedge) {
        return TRUE;
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
                if (status) *status |= EDGE_CHECK_COEDGE_SENSE_ERROR;
                valid = FALSE;
            }
        }

        coedge = coedge->next();
    } while (coedge && coedge != first_coedge);

    return valid;
}

logical check_edge_evaluation(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    CURVE *curve = edge->curfi();
    if (!curve) {
        return TRUE;
    }

    logical valid = TRUE;
    int num_samples = 15;
    SPAinterval range = edge->param_range();

    for (int i = 0; i <= num_samples; i++) {
        double t = range.low() +
                   (range.high() - range.low()) * i / num_samples;

        try {
            SPAposition pos = curve->eval_position(t);

            if (std::isnan(pos.x()) || std::isnan(pos.y()) ||
                std::isnan(pos.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Edge curve evaluation returned NaN.");
                ilist->add(id);
                if (status) *status |= EDGE_CHECK_EVAL_FAILURE;
                return FALSE;
            }

            if (std::isinf(pos.x()) || std::isinf(pos.y()) ||
                std::isinf(pos.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Edge curve evaluation returned Inf.");
                ilist->add(id);
                if (status) *status |= EDGE_CHECK_EVAL_FAILURE;
                return FALSE;
            }
        } catch (...) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Edge curve evaluation threw exception.");
            ilist->add(id);
            if (status) *status |= EDGE_CHECK_EVAL_FAILURE;
            return FALSE;
        }
    }

    return valid;
}

logical check_edge_fit_tolerance(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    logical valid = TRUE;
    double fit_tol = edge->fit_tolerance();

    if (std::isnan(fit_tol) || std::isinf(fit_tol)) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge fit tolerance is NaN or Inf.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_FIT_TOLERANCE;
        valid = FALSE;
    }

    if (fit_tol < 0) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge fit tolerance is negative.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_FIT_TOLERANCE;
        valid = FALSE;
    }

    if (fit_tol > SPAresabs * 1000) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Edge fit tolerance is unusually large.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_FIT_TOLERANCE;
    }

    return valid;
}

logical check_edge_length(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    logical valid = TRUE;

    VERTEX *v_start = edge->start();
    VERTEX *v_end = edge->end();

    if (!v_start || !v_end) {
        return TRUE;
    }

    POINT *pt_start = v_start->point();
    POINT *pt_end = v_end->point();

    if (!pt_start || !pt_end) {
        return TRUE;
    }

    SPAposition start_pos = pt_start->position();
    SPAposition end_pos = pt_end->position();
    double length = (end_pos - start_pos).length();

    if (std::isnan(length) || std::isinf(length)) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge computed length is NaN or Inf.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_LENGTH;
        valid = FALSE;
    }

    return valid;
}

logical check_edge_g1_continuity(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    CURVE *curve = edge->curfi();
    if (!curve) {
        return TRUE;
    }

    logical valid = TRUE;
    SPAinterval range = edge->param_range();
    int num_samples = 10;
    double eps = (range.high() - range.low()) * 0.001;

    // Check interior G1 continuity by sampling tangent vectors
    for (int i = 0; i < num_samples; i++) {
        double t1 = range.low() + (range.high() - range.low()) * i / num_samples;
        double t2 = range.low() + (range.high() - range.low()) * (i + 1) / num_samples;

        try {
            SPAvector tan1 = curve->eval_deriv(t1 + eps);
            SPAvector tan2 = curve->eval_deriv(t2 - eps);

            if (tan1.length() > SPAresabs && tan2.length() > SPAresabs) {
                double cos_angle = (tan1 | tan2) /
                                   (tan1.length() * tan2.length());

                if (cos_angle < 1.0 - SPAresnor * 10) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(WARNING);
                    id->set_description(
                        "Edge G1 discontinuity detected at interior."
                    );
                    ilist->add(id);
                    if (status) *status |= EDGE_CHECK_NON_G1_CONTINUITY;
                    valid = FALSE;
                    break;
                }
            }
        } catch (...) {
            // Skip on exception
        }
    }

    // Additional check at closure seam for closed edges
    if (valid && edge->closed()) {
        try {
            SPAvector tan_start = curve->eval_deriv(range.low() + eps);
            SPAvector tan_end = curve->eval_deriv(range.high() - eps);

            if (tan_start.length() > SPAresabs &&
                tan_end.length() > SPAresabs) {
                double cos_angle = (tan_start | tan_end) /
                                   (tan_start.length() * tan_end.length());

                if (cos_angle < 1.0 - SPAresnor * 10) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(WARNING);
                    id->set_description(
                        "Edge G1 discontinuity at closure seam."
                    );
                    ilist->add(id);
                    if (status) *status |= EDGE_CHECK_NON_G1_CONTINUITY;
                    valid = FALSE;
                }
            }
        } catch (...) {
            // Skip on exception
        }
    }

    return valid;
}

logical check_edge_bounding_box(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    logical valid = TRUE;

    VERTEX *v_start = edge->start();
    VERTEX *v_end = edge->end();

    if (v_start && v_start->point()) {
        SPAposition pos = v_start->point()->position();
        if (std::isnan(pos.x()) || std::isnan(pos.y()) || std::isnan(pos.z())) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Edge start vertex has NaN in bounding box.");
            ilist->add(id);
            if (status) *status |= EDGE_CHECK_BAD_BOUNDING_BOX;
            valid = FALSE;
        }
        if (std::isinf(pos.x()) || std::isinf(pos.y()) || std::isinf(pos.z())) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Edge start vertex has Inf in bounding box.");
            ilist->add(id);
            if (status) *status |= EDGE_CHECK_BAD_BOUNDING_BOX;
            valid = FALSE;
        }
    }

    if (v_end && v_end->point()) {
        SPAposition pos = v_end->point()->position();
        if (std::isnan(pos.x()) || std::isnan(pos.y()) || std::isnan(pos.z())) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Edge end vertex has NaN in bounding box.");
            ilist->add(id);
            if (status) *status |= EDGE_CHECK_BAD_BOUNDING_BOX;
            valid = FALSE;
        }
        if (std::isinf(pos.x()) || std::isinf(pos.y()) || std::isinf(pos.z())) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("Edge end vertex has Inf in bounding box.");
            ilist->add(id);
            if (status) *status |= EDGE_CHECK_BAD_BOUNDING_BOX;
            valid = FALSE;
        }
    }

    return valid;
}

logical check_edge_param_normalization(
    EDGE          *edge,
    insanity_list *ilist,
    int           *status
) {
    if (!edge) {
        return FALSE;
    }

    logical valid = TRUE;

    double start_param = edge->start_param();
    double end_param = edge->end_param();

    if (std::isnan(start_param) || std::isnan(end_param)) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge parameter values contain NaN.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_PARAM_NORMALIZATION;
        valid = FALSE;
    }

    if (std::isinf(start_param) || std::isinf(end_param)) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Edge parameter values contain Inf.");
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_PARAM_NORMALIZATION;
        valid = FALSE;
    }

    if (!edge->closed() && start_param > end_param) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description(
            "Edge start parameter > end parameter (non-closed)."
        );
        ilist->add(id);
        if (status) *status |= EDGE_CHECK_BAD_PARAM_NORMALIZATION;
    }

    return valid;
}

int api_check_edge(
    EDGE *edge,
    int  *insanity_count
) {
    if (!edge || edge->identity() != EDGE_TYPE) {
        return EDGE_CHECK_NULL_EDGE;
    }

    insanity_list ilist;
    int count = 0;
    int status = EDGE_CHECK_OK;

    if (check_edge_null(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_curve(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_vertices(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_degenerate(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_parameter_range(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_vertex_on_curve(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_closure(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_coedge_sense(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_evaluation(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_fit_tolerance(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_length(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_g1_continuity(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_bounding_box(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_edge_param_normalization(edge, &ilist, &status) == FALSE) {
        count++;
    }

    if (insanity_count) {
        *insanity_count = count;
    }

<<<<<<< HEAD
    int status = EDGE_CHECK_OK;
    insanity_data *entry = ilist.first();
    while (entry) {
        const char *desc = entry->get_description();
        if (desc) {
            if (strstr(desc, "null")) {
                if (strstr(desc, "curve")) {
                    status |= EDGE_CHECK_NULL_CURVE;
                } else if (strstr(desc, "vertex")) {
                    status |= EDGE_CHECK_NULL_VERTEX;
                } else if (strstr(desc, "Edge")) {
                    status |= EDGE_CHECK_NULL_EDGE;
                }
            }
            if (strstr(desc, "degenerate")) {
                status |= EDGE_CHECK_DEGENERATE;
            }
            if (strstr(desc, "parameter")) {
                status |= EDGE_CHECK_BAD_PARAM_RANGE;
            }
            if (strstr(desc, "not at curve")) {
                status |= EDGE_CHECK_VERTEX_NOT_ON_CURVE;
            }
            if (strstr(desc, "closure") || strstr(desc, "closed")) {
                status |= EDGE_CHECK_BAD_CLOSURE;
            }
            if (strstr(desc, "sense")) {
                status |= EDGE_CHECK_COEDGE_SENSE_ERROR;
            }
            if (strstr(desc, "evaluation") || strstr(desc, "threw")) {
                status |= EDGE_CHECK_EVAL_FAILURE;
            }
            if (strstr(desc, "NaN") || strstr(desc, "Inf")) {
                status |= EDGE_CHECK_NAN_COORDINATES;
            }
            if (strstr(desc, "tolerance")) {
                status |= EDGE_CHECK_BAD_FIT_TOLERANCE;
            }
            if (strstr(desc, "length")) {
                status |= EDGE_CHECK_BAD_LENGTH;
            }
            if (strstr(desc, "G1")) {
                status |= EDGE_CHECK_NON_G1_CONTINUITY;
            }
            if (strstr(desc, "bounding box")) {
                status |= EDGE_CHECK_BAD_BOUNDING_BOX;
            }
            if (strstr(desc, "normalization") ||
                strstr(desc, "start parameter > end parameter")) {
                status |= EDGE_CHECK_BAD_PARAM_NORMALIZATION;
            }
        }

        entry = entry->next();
    }

=======
>>>>>>> 5b669d23a53d60b412b23abc0b2023797e15d235
    return status;
}
