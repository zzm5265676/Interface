#include "bs3_curve_check.hxx"
#include "curve.hxx"
#include "bs3_curve.hxx"
#include "SPApar.hxx"
#include "SPAres.hxx"
#include "SPAposition.hxx"
#include "SPAvector.hxx"
#include <cmath>
#include <cstring>

bs3_curve_check_result::bs3_curve_check_result()
    : _status(BS3_CURVE_CHECK_OK),
      _eval_failure_count(0),
      _degenerate_count(0) {
}

bs3_curve_check_result::~bs3_curve_check_result() {
}

int bs3_curve_check_result::get_status() const {
    return _status;
}

bool bs3_curve_check_result::is_ok() const {
    return _status == BS3_CURVE_CHECK_OK;
}

void bs3_curve_check_result::set_status(int status) {
    _status = status;
}

int bs3_curve_check_result::eval_failure_count() const {
    return _eval_failure_count;
}

int bs3_curve_check_result::degenerate_count() const {
    return _degenerate_count;
}

void bs3_curve_check_result::note_eval_failure() {
    ++_eval_failure_count;
}

void bs3_curve_check_result::note_degenerate() {
    ++_degenerate_count;
}

void bs3_curve_check_result::add_insanity(insanity_data *data) {
    if (data) {
        _insanities.add(data);
    }
}

insanity_list *bs3_curve_check_result::get_insanity_list() {
    return &_insanities;
}

outcome api_bs3_curve_check(
    BS3_CURVE            *curve,
    bs3_curve_check_result &result,
    AcisOptions          *ao
) {
    outcome res = outcome::success;

    if (!curve) {
        return outcome(API_NULL_ARGUMENT, FAIL, 0);
    }

    check_bs3_curve_null(curve, result.get_insanity_list());

    check_bs3_curve_order(curve, result.get_insanity_list());

    check_bs3_curve_control_points(curve, result.get_insanity_list());

    check_bs3_curve_knot_vector(curve, result.get_insanity_list());

    check_bs3_curve_evaluation(curve, result.get_insanity_list());

    check_bs3_curve_parameter_range(curve, result.get_insanity_list());

    check_bs3_curve_closure(curve, result.get_insanity_list());

    check_bs3_curve_fit_tolerance(curve, result.get_insanity_list());

    check_bs3_curve_degeneracy(curve, result.get_insanity_list());

    check_bs3_curve_derivatives(curve, result.get_insanity_list());

    check_bs3_curve_knot_multiplicity(curve, result.get_insanity_list());

    check_bs3_curve_convex_hull(curve, result.get_insanity_list());

    check_bs3_curve_vd_property(curve, result.get_insanity_list());

    check_bs3_curve_bounding_box(curve, result.get_insanity_list());

    check_bs3_curve_arc_length(curve, result.get_insanity_list());

    int status = BS3_CURVE_CHECK_OK;
    insanity_data *entry = result.get_insanity_list()->first();
    while (entry) {
        const char *desc = entry->get_description();
        if (desc) {
            if (strstr(desc, "null")) {
                status |= BS3_CURVE_CHECK_NULL_CURVE;
            }
            if (strstr(desc, "order")) {
                status |= BS3_CURVE_CHECK_BAD_ORDER;
            }
            if (strstr(desc, "knot")) {
                status |= BS3_CURVE_CHECK_BAD_KNOT_VECTOR;
            }
            if (strstr(desc, "control point")) {
                status |= BS3_CURVE_CHECK_BAD_CP_COUNT;
            }
            if (strstr(desc, "coincident")) {
                status |= BS3_CURVE_CHECK_COINCIDENT_CPS;
            }
            if (strstr(desc, "evaluation") || strstr(desc, "threw")) {
                status |= BS3_CURVE_CHECK_EVAL_FAILURE;
                result.note_eval_failure();
            }
            if (strstr(desc, "NaN") || strstr(desc, "Inf")) {
                status |= BS3_CURVE_CHECK_NAN_COORDINATES;
            }
            if (strstr(desc, "parameter")) {
                status |= BS3_CURVE_CHECK_BAD_PARAM_RANGE;
            }
            if (strstr(desc, "closure") || strstr(desc, "closed")) {
                status |= BS3_CURVE_CHECK_BAD_CLOSURE;
            }
            if (strstr(desc, "tolerance")) {
                status |= BS3_CURVE_CHECK_BAD_FIT_TOL;
            }
            if (strstr(desc, "degenerate")) {
                status |= BS3_CURVE_CHECK_DEGENERATE;
                result.note_degenerate();
            }
            if (strstr(desc, "multiplicity")) {
                status |= BS3_CURVE_CHECK_BAD_KNOT_MULT;
            }
            if (strstr(desc, "convex hull")) {
                status |= BS3_CURVE_CHECK_BAD_CONVEX_HULL;
            }
            if (strstr(desc, "variation diminishing")) {
                status |= BS3_CURVE_CHECK_BAD_VD_PROPERTY;
            }
            if (strstr(desc, "bounding box")) {
                status |= BS3_CURVE_CHECK_BAD_BOUNDING_BOX;
            }
            if (strstr(desc, "arc length")) {
                status |= BS3_CURVE_CHECK_BAD_ARC_LENGTH;
            }
        }
        entry = entry->next();
    }
    result.set_status(status);

    return res;
}

logical check_bs3_curve_null(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("BS3_CURVE pointer is null.");
        ilist->add(id);
        return FALSE;
    }

    return TRUE;
}

logical check_bs3_curve_order(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    int order = curve->order();

    if (order < 1) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("BS3_CURVE order is less than 1.");
        ilist->add(id);
        return FALSE;
    }

    if (order > 20) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("BS3_CURVE order is unusually high.");
        ilist->add(id);
    }

    return TRUE;
}

logical check_bs3_curve_control_points(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_cp = curve->num_control_points();
    int order = curve->order();

    if (num_cp < order) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("BS3_CURVE: control points < order.");
        ilist->add(id);
        return FALSE;
    }

    if (num_cp < 2) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("BS3_CURVE has only 1 control point.");
        ilist->add(id);
        return FALSE;
    }

    for (int i = 0; i < num_cp; i++) {
        SPAposition cp = curve->control_point(i);
        if (std::isnan(cp.x()) || std::isnan(cp.y()) || std::isnan(cp.z())) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("BS3_CURVE control point has NaN coordinates.");
            ilist->add(id);
            valid = FALSE;
            break;
        }
        if (std::isinf(cp.x()) || std::isinf(cp.y()) || std::isinf(cp.z())) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("BS3_CURVE control point has Inf coordinates.");
            ilist->add(id);
            valid = FALSE;
            break;
        }
    }

    return valid;
}

logical check_bs3_curve_knot_vector(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_cp = curve->num_control_points();
    int order = curve->order();
    int num_knots = num_cp + order;

    if (!curve->knot_vector()) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("BS3_CURVE has null knot vector.");
        ilist->add(id);
        return FALSE;
    }

    double prev_knot = curve->knot(0);
    for (int i = 1; i < num_knots; i++) {
        double k = curve->knot(i);
        if (k < prev_knot - SPAresabs) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("BS3_CURVE knot vector is not non-decreasing.");
            ilist->add(id);
            valid = FALSE;
            break;
        }
        prev_knot = k;
    }

    if (valid) {
        for (int i = 0; i < num_knots; i++) {
            double k = curve->knot(i);
            if (std::isnan(k) || std::isinf(k)) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("BS3_CURVE knot contains NaN/Inf.");
                ilist->add(id);
                valid = FALSE;
                break;
            }
        }
    }

    return valid;
}

logical check_bs3_curve_evaluation(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_samples = 20;
    SPAinterval range = curve->param_range();

    for (int i = 0; i <= num_samples; i++) {
        double t = range.low() +
                   (range.high() - range.low()) * i / num_samples;

        try {
            SPAposition pos = curve->eval_position(t);

            if (std::isnan(pos.x()) || std::isnan(pos.y()) ||
                std::isnan(pos.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("BS3_CURVE evaluation returned NaN.");
                ilist->add(id);
                valid = FALSE;
                return valid;
            }

            if (std::isinf(pos.x()) || std::isinf(pos.y()) ||
                std::isinf(pos.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("BS3_CURVE evaluation returned Inf.");
                ilist->add(id);
                valid = FALSE;
                return valid;
            }
        } catch (...) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description("BS3_CURVE evaluation threw exception.");
            ilist->add(id);
            valid = FALSE;
            return valid;
        }
    }

    return valid;
}

logical check_bs3_curve_parameter_range(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    SPAinterval range = curve->param_range();

    if (range.null()) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("BS3_CURVE parameter range is null.");
        ilist->add(id);
        return FALSE;
    }

    if (range.high() - range.low() < SPAresabs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("BS3_CURVE parameter range is degenerate.");
        ilist->add(id);
    }

    if (std::isnan(range.low()) || std::isnan(range.high())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("BS3_CURVE parameter range contains NaN.");
        ilist->add(id);
        return FALSE;
    }

    if (std::isinf(range.low()) || std::isinf(range.high())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("BS3_CURVE parameter range contains Inf.");
        ilist->add(id);
        return FALSE;
    }

    return TRUE;
}

logical check_bs3_curve_closure(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    if (!curve->closed()) {
        return TRUE;
    }

    logical valid = TRUE;
    SPAinterval range = curve->param_range();

    SPAposition start_pos = curve->eval_position(range.low());
    SPAposition end_pos = curve->eval_position(range.high());

    if ((start_pos - end_pos).length() > SPAresabs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description(
            "BS3_CURVE marked closed but start/end positions differ."
        );
        ilist->add(id);
        valid = FALSE;
    }

    double mid_t = (range.low() + range.high()) / 2.0;
    SPAvector start_tan = curve->eval_deriv(range.low());
    SPAvector end_tan = curve->eval_deriv(range.high());

    if (start_tan.length() > SPAresabs && end_tan.length() > SPAresabs) {
        double cos_angle = (start_tan | end_tan) /
                           (start_tan.length() * end_tan.length());
        if (cos_angle < 1.0 - SPAresnor) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(WARNING);
            id->set_description(
                "BS3_CURVE closed but tangents mismatch at seam."
            );
            ilist->add(id);
            valid = FALSE;
        }
    }

    return valid;
}

logical check_bs3_curve_fit_tolerance(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    double fit_tol = curve->fit_tolerance();

    if (fit_tol < 0) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("BS3_CURVE fit tolerance is negative.");
        ilist->add(id);
        valid = FALSE;
    }

    if (fit_tol > SPAresabs * 1000) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("BS3_CURVE fit tolerance is unusually large.");
        ilist->add(id);
    }

    return valid;
}

logical check_bs3_curve_degeneracy(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_cp = curve->num_control_points();

    if (num_cp < 2) {
        return TRUE;
    }

    SPAposition cp0 = curve->control_point(0);
    bool all_coincident = true;

    for (int i = 1; i < num_cp; i++) {
        SPAposition cp = curve->control_point(i);
        if ((cp - cp0).length() > SPAresabs) {
            all_coincident = false;
            break;
        }
    }

    if (all_coincident) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description(
            "BS3_CURVE: all control points are coincident (degenerate)."
        );
        ilist->add(id);
        valid = FALSE;
    }

    int consecutive_coincident = 0;
    int max_consecutive = 0;

    for (int i = 1; i < num_cp; i++) {
        SPAposition prev = curve->control_point(i - 1);
        SPAposition curr = curve->control_point(i);

        if ((curr - prev).length() < SPAresabs) {
            consecutive_coincident++;
            if (consecutive_coincident > max_consecutive) {
                max_consecutive = consecutive_coincident;
            }
        } else {
            consecutive_coincident = 0;
        }
    }

    int order = curve->order();
    if (max_consecutive >= order - 1) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description(
            "BS3_CURVE: too many consecutive coincident control points."
        );
        ilist->add(id);
        valid = FALSE;
    }

    return valid;
}

logical check_bs3_curve_derivatives(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_samples = 15;
    SPAinterval range = curve->param_range();

    for (int i = 0; i <= num_samples; i++) {
        double t = range.low() +
                   (range.high() - range.low()) * i / num_samples;

        try {
            SPAvector deriv = curve->eval_deriv(t);

            if (std::isnan(deriv.x()) || std::isnan(deriv.y()) ||
                std::isnan(deriv.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description(
                    "BS3_CURVE first derivative evaluation returned NaN."
                );
                ilist->add(id);
                valid = FALSE;
                return valid;
            }

            if (std::isinf(deriv.x()) || std::isinf(deriv.y()) ||
                std::isinf(deriv.z())) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description(
                    "BS3_CURVE first derivative evaluation returned Inf."
                );
                ilist->add(id);
                valid = FALSE;
                return valid;
            }
        } catch (...) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description(
                "BS3_CURVE derivative evaluation threw exception."
            );
            ilist->add(id);
            valid = FALSE;
            return valid;
        }
    }

    if (!curve->closed()) {
        SPAvector deriv_start = curve->eval_deriv(range.low());
        SPAvector deriv_end = curve->eval_deriv(range.high());

        if (deriv_start.length() < SPAresabs &&
            deriv_end.length() < SPAresabs) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(WARNING);
            id->set_description(
                "BS3_CURVE: zero derivatives at both endpoints."
            );
            ilist->add(id);
            valid = FALSE;
        }
    }

    return valid;
}

logical check_bs3_curve_knot_multiplicity(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_cp = curve->num_control_points();
    int order = curve->order();
    int num_knots = num_cp + order;

    double max_mult = order;
    int current_mult = 1;

    for (int i = 1; i < num_knots; i++) {
        double k_prev = curve->knot(i - 1);
        double k_curr = curve->knot(i);

        if (fabs(k_curr - k_prev) < SPAresabs) {
            current_mult++;
            if (current_mult > max_mult) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description(
                    "BS3_CURVE knot multiplicity exceeds order."
                );
                ilist->add(id);
                valid = FALSE;
                break;
            }
        } else {
            current_mult = 1;
        }
    }

    return valid;
}

logical check_bs3_curve_convex_hull(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_cp = curve->num_control_points();

    if (num_cp < 2) {
        return TRUE;
    }

    SPAposition cp0 = curve->control_point(0);
    SPAposition cp_min = cp0;
    SPAposition cp_max = cp0;

    for (int i = 1; i < num_cp; i++) {
        SPAposition cp = curve->control_point(i);

        if (cp.x() < cp_min.x()) cp_min = SPAposition(cp.x(), cp_min.y(), cp_min.z());
        if (cp.y() < cp_min.y()) cp_min = SPAposition(cp_min.x(), cp.y(), cp_min.z());
        if (cp.z() < cp_min.z()) cp_min = SPAposition(cp_min.x(), cp_min.y(), cp.z());

        if (cp.x() > cp_max.x()) cp_max = SPAposition(cp.x(), cp_max.y(), cp_max.z());
        if (cp.y() > cp_max.y()) cp_max = SPAposition(cp_max.x(), cp.y(), cp_max.z());
        if (cp.z() > cp_max.z()) cp_max = SPAposition(cp_max.x(), cp_max.y(), cp.z());
    }

    int num_samples = 20;
    SPAinterval range = curve->param_range();

    for (int i = 0; i <= num_samples; i++) {
        double t = range.low() +
                   (range.high() - range.low()) * i / num_samples;

        try {
            SPAposition pos = curve->eval_position(t);

            if (pos.x() < cp_min.x() - SPAresabs ||
                pos.y() < cp_min.y() - SPAresabs ||
                pos.z() < cp_min.z() - SPAresabs) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(WARNING);
                id->set_description(
                    "BS3_CURVE point outside convex hull (min)."
                );
                ilist->add(id);
                valid = FALSE;
                break;
            }

            if (pos.x() > cp_max.x() + SPAresabs ||
                pos.y() > cp_max.y() + SPAresabs ||
                pos.z() > cp_max.z() + SPAresabs) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(WARNING);
                id->set_description(
                    "BS3_CURVE point outside convex hull (max)."
                );
                ilist->add(id);
                valid = FALSE;
                break;
            }
        } catch (...) {
            // Skip on exception
        }
    }

    return valid;
}

logical check_bs3_curve_vd_property(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_cp = curve->num_control_points();

    if (num_cp < 3) {
        return TRUE;
    }

    int num_samples = 30;
    SPAinterval range = curve->param_range();

    SPAposition prev_pos = curve->eval_position(range.low());

    for (int i = 1; i <= num_samples; i++) {
        double t = range.low() +
                   (range.high() - range.low()) * i / num_samples;

        try {
            SPAposition curr_pos = curve->eval_position(t);

            SPAvector prev_tan = curve->eval_deriv(
                range.low() + (range.high() - range.low()) * (i - 1) / num_samples
            );
            SPAvector curr_tan = curve->eval_deriv(t);

            if (prev_tan.length() > SPAresabs &&
                curr_tan.length() > SPAresabs) {
                double cos_angle = (prev_tan | curr_tan) /
                                   (prev_tan.length() * curr_tan.length());

                if (cos_angle < -0.9) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(WARNING);
                    id->set_description(
                        "BS3_CURVE may violate variation diminishing property."
                    );
                    ilist->add(id);
                    valid = FALSE;
                    break;
                }
            }

            prev_pos = curr_pos;
        } catch (...) {
            // Skip on exception
        }
    }

    return valid;
}

logical check_bs3_curve_bounding_box(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_cp = curve->num_control_points();

    for (int i = 0; i < num_cp; i++) {
        SPAposition cp = curve->control_point(i);

        if (std::isnan(cp.x()) || std::isnan(cp.y()) || std::isnan(cp.z())) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description(
                "BS3_CURVE control point has NaN in bounding box."
            );
            ilist->add(id);
            valid = FALSE;
            break;
        }

        if (std::isinf(cp.x()) || std::isinf(cp.y()) || std::isinf(cp.z())) {
            insanity_data *id = new insanity_data();
            id->set_insanity_type(ERROR_TYPE);
            id->set_description(
                "BS3_CURVE control point has Inf in bounding box."
            );
            ilist->add(id);
            valid = FALSE;
            break;
        }
    }

    return valid;
}

logical check_bs3_curve_arc_length(
    BS3_CURVE       *curve,
    insanity_list   *ilist
) {
    if (!curve) {
        return FALSE;
    }

    logical valid = TRUE;

    int num_samples = 50;
    SPAinterval range = curve->param_range();
    double total_length = 0.0;

    SPAposition prev_pos = curve->eval_position(range.low());

    for (int i = 1; i <= num_samples; i++) {
        double t = range.low() +
                   (range.high() - range.low()) * i / num_samples;

        try {
            SPAposition curr_pos = curve->eval_position(t);
            double seg_len = (curr_pos - prev_pos).length();
            total_length += seg_len;
            prev_pos = curr_pos;
        } catch (...) {
            // Skip on exception
        }
    }

    if (total_length < SPAresabs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description(
            "BS3_CURVE arc length is near zero (degenerate)."
        );
        ilist->add(id);
        valid = FALSE;
    }

    if (std::isnan(total_length) || std::isinf(total_length)) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description(
            "BS3_CURVE arc length is NaN or Inf."
        );
        ilist->add(id);
        valid = FALSE;
    }

    return valid;
}

int bs3_curve_check(
    BS3_CURVE *curve,
    int       *insanity_count
) {
    if (!curve) {
        return BS3_CURVE_CHECK_NULL_CURVE;
    }

    insanity_list ilist;
    int count = 0;

    if (check_bs3_curve_null(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_order(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_control_points(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_knot_vector(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_evaluation(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_parameter_range(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_closure(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_fit_tolerance(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_degeneracy(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_derivatives(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_knot_multiplicity(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_convex_hull(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_vd_property(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_bounding_box(curve, &ilist) == FALSE) {
        count++;
    }

    if (check_bs3_curve_arc_length(curve, &ilist) == FALSE) {
        count++;
    }

    if (insanity_count) {
        *insanity_count = count;
    }

    int status = BS3_CURVE_CHECK_OK;
    insanity_data *entry = ilist.first();
    while (entry) {
        const char *desc = entry->get_description();
        if (desc) {
            if (strstr(desc, "null")) {
                status |= BS3_CURVE_CHECK_NULL_CURVE;
            }
            if (strstr(desc, "order")) {
                status |= BS3_CURVE_CHECK_BAD_ORDER;
            }
            if (strstr(desc, "knot")) {
                status |= BS3_CURVE_CHECK_BAD_KNOT_VECTOR;
            }
            if (strstr(desc, "control point")) {
                status |= BS3_CURVE_CHECK_BAD_CP_COUNT;
            }
            if (strstr(desc, "coincident")) {
                status |= BS3_CURVE_CHECK_COINCIDENT_CPS;
            }
            if (strstr(desc, "evaluation") || strstr(desc, "threw")) {
                status |= BS3_CURVE_CHECK_EVAL_FAILURE;
            }
            if (strstr(desc, "NaN") || strstr(desc, "Inf")) {
                status |= BS3_CURVE_CHECK_NAN_COORDINATES;
            }
            if (strstr(desc, "parameter")) {
                status |= BS3_CURVE_CHECK_BAD_PARAM_RANGE;
            }
            if (strstr(desc, "closure") || strstr(desc, "closed")) {
                status |= BS3_CURVE_CHECK_BAD_CLOSURE;
            }
            if (strstr(desc, "tolerance")) {
                status |= BS3_CURVE_CHECK_BAD_FIT_TOL;
            }
            if (strstr(desc, "degenerate")) {
                status |= BS3_CURVE_CHECK_DEGENERATE;
            }
            if (strstr(desc, "multiplicity")) {
                status |= BS3_CURVE_CHECK_BAD_KNOT_MULT;
            }
            if (strstr(desc, "convex hull")) {
                status |= BS3_CURVE_CHECK_BAD_CONVEX_HULL;
            }
            if (strstr(desc, "variation diminishing")) {
                status |= BS3_CURVE_CHECK_BAD_VD_PROPERTY;
            }
            if (strstr(desc, "bounding box")) {
                status |= BS3_CURVE_CHECK_BAD_BOUNDING_BOX;
            }
            if (strstr(desc, "arc length")) {
                status |= BS3_CURVE_CHECK_BAD_ARC_LENGTH;
            }
        }

        entry = entry->next();
    }

    return status;
}
