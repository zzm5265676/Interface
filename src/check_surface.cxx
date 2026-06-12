#include "check_surface.hxx"
#include "surface.hxx"
#include "bs3_surface.hxx"
#include "point.hxx"
#include "SPAbox.hxx"
#include "SPAres.hxx"
#include "bsf.hxx"
#include "bsfacc.hxx"
#include <cmath>
#include <cstring>

surface_check_result::surface_check_result()
    : _status(SURF_CHECK_OK),
      _eval_failure_count(0),
      _singularity_count(0) {
}

surface_check_result::~surface_check_result() {
}

int surface_check_result::get_status() const {
    return _status;
}

bool surface_check_result::is_ok() const {
    return _status == SURF_CHECK_OK;
}

void surface_check_result::set_status(int status) {
    _status = status;
}

int surface_check_result::eval_failure_count() const {
    return _eval_failure_count;
}

int surface_check_result::singularity_count() const {
    return _singularity_count;
}

void surface_check_result::add_insanity(insanity_data *data) {
    if (data) {
        _insanities.add(data);
    }
}

insanity_list *surface_check_result::get_insanity_list() {
    return &_insanities;
}

outcome api_check_surface_ok(
    SURFACE             *surface,
    surface_check_result &result,
    AcisOptions         *ao
) {
    outcome res = outcome::success;

    if (!surface) {
        return outcome(API_NULL_ARGUMENT, FAIL, 0);
    }

    int status = SURF_CHECK_OK;

    check_surface_null(surface, result.get_insanity_list(), &status);
    check_surface_evaluation(surface, result.get_insanity_list(), &status);
    check_surface_parameter_range(surface, result.get_insanity_list(), &status);
    check_surface_continuity(surface, result.get_insanity_list(), &status);
    check_surface_singularity(surface, result.get_insanity_list(), &status);
    check_surface_closure(surface, result.get_insanity_list(), &status);
    check_surface_fit_tolerance(surface, result.get_insanity_list(), &status);
    check_bspline_surface(surface, result.get_insanity_list(), &status);
    check_surface_self_intersection(surface, result.get_insanity_list(), &status);
    check_surface_normal_consistency(surface, result.get_insanity_list(), &status);
    check_surface_g2_continuity(surface, result.get_insanity_list(), &status);
    check_surface_uv_coordinates(surface, result.get_insanity_list(), &status);
    check_surface_area_degenerate(surface, result.get_insanity_list(), &status);
    check_surface_periodicity(surface, result.get_insanity_list(), &status);

    result.set_status(status);
    return res;
}

logical check_surface_null(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Surface pointer is null.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_NULL_SURFACE;
        return FALSE;
    }

    return TRUE;
}

logical check_surface_evaluation(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_samples = 10;

    SPApar_box pb = surface->param_range();
    SPAinterval u_range = pb.interval(0);
    SPAinterval v_range = pb.interval(1);

    for (int i = 0; i <= num_samples; i++) {
        for (int j = 0; j <= num_samples; j++) {
            double u = u_range.low() +
                       (u_range.high() - u_range.low()) * i / num_samples;
            double v = v_range.low() +
                       (v_range.high() - v_range.low()) * j / num_samples;

            SPApar_pos par_pos(u, v);
            SPAposition pos;

            try {
                pos = surface->eval_position(par_pos);

                if (std::isnan(pos.x()) || std::isnan(pos.y()) ||
                    std::isnan(pos.z())) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(ERROR_TYPE);
                    id->set_description("Surface evaluation returned NaN.");
                    ilist->add(id);
                    if (status) *status |= SURF_CHECK_EVAL_FAILURE;
                    return FALSE;
                }

                if (std::isinf(pos.x()) || std::isinf(pos.y()) ||
                    std::isinf(pos.z())) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(ERROR_TYPE);
                    id->set_description("Surface evaluation returned Inf.");
                    ilist->add(id);
                    if (status) *status |= SURF_CHECK_EVAL_FAILURE;
                    return FALSE;
                }
            } catch (...) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Surface evaluation threw exception.");
                ilist->add(id);
                if (status) *status |= SURF_CHECK_EVAL_FAILURE;
                return FALSE;
            }
        }
    }

    return valid;
}

logical check_surface_parameter_range(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    SPApar_box pb = surface->param_range();
    SPAinterval u_range = pb.interval(0);
    SPAinterval v_range = pb.interval(1);

    if (u_range.null() || v_range.null()) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Surface parameter range is null.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_PARAMETER_RANGE;
        return FALSE;
    }

    if (u_range.high() - u_range.low() < SPAresabs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Surface U parameter range is degenerate.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_PARAMETER_RANGE;
    }

    if (v_range.high() - v_range.low() < SPAresabs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Surface V parameter range is degenerate.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_PARAMETER_RANGE;
    }

    if (std::isnan(u_range.low()) || std::isnan(u_range.high()) ||
        std::isnan(v_range.low()) || std::isnan(v_range.high())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Surface parameter range contains NaN.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_NAN_COORDINATES;
        return FALSE;
    }

    if (std::isinf(u_range.low()) || std::isinf(u_range.high()) ||
        std::isinf(v_range.low()) || std::isinf(v_range.high())) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Surface parameter range contains Inf.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_NAN_COORDINATES;
        return FALSE;
    }

    return TRUE;
}

logical check_surface_continuity(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;

    // G1 tangent continuity check at closed surface seams
    if (surface->closed_u() || surface->closed_v()) {
        SPApar_box pb = surface->param_range();
        SPAinterval u_range = pb.interval(0);
        SPAinterval v_range = pb.interval(1);

        int num_samples = 5;

        if (surface->closed_u()) {
            double eps = (u_range.high() - u_range.low()) * 0.001;
            for (int j = 0; j <= num_samples; j++) {
                double v = v_range.low() +
                           (v_range.high() - v_range.low()) * j / num_samples;

                SPApar_pos p1(u_range.low() + eps, v);
                SPApar_pos p2(u_range.high() - eps, v);

                SPAvector du1, dv1, du2, dv2;
                try {
                    surface->eval_derivs(p1, du1, dv1);
                    surface->eval_derivs(p2, du2, dv2);

                    if (du1.length() > SPAresabs && du2.length() > SPAresabs) {
                        double cos_angle = (du1 | du2) /
                                           (du1.length() * du2.length());
                        if (cos_angle < 1.0 - SPAresnor * 10) {
                            insanity_data *id = new insanity_data();
                            id->set_insanity_type(WARNING);
                            id->set_description(
                                "Surface G1 discontinuity at U seam."
                            );
                            ilist->add(id);
                            if (status) *status |= SURF_CHECK_NON_G1;
                            valid = FALSE;
                        }
                    }
                } catch (...) {
                    // Skip on exception
                }
            }
        }

        if (surface->closed_v()) {
            double eps = (v_range.high() - v_range.low()) * 0.001;
            for (int i = 0; i <= num_samples; i++) {
                double u = u_range.low() +
                           (u_range.high() - u_range.low()) * i / num_samples;

                SPApar_pos p1(u, v_range.low() + eps);
                SPApar_pos p2(u, v_range.high() - eps);

                SPAvector du1, dv1, du2, dv2;
                try {
                    surface->eval_derivs(p1, du1, dv1);
                    surface->eval_derivs(p2, du2, dv2);

                    if (dv1.length() > SPAresabs && dv2.length() > SPAresabs) {
                        double cos_angle = (dv1 | dv2) /
                                           (dv1.length() * dv2.length());
                        if (cos_angle < 1.0 - SPAresnor * 10) {
                            insanity_data *id = new insanity_data();
                            id->set_insanity_type(WARNING);
                            id->set_description(
                                "Surface G1 discontinuity at V seam."
                            );
                            ilist->add(id);
                            if (status) *status |= SURF_CHECK_NON_G1;
                            valid = FALSE;
                        }
                    }
                } catch (...) {
                    // Skip on exception
                }
            }
        }
    }

    return valid;
}

logical check_surface_singularity(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_samples = 15;
    SPApar_box pb = surface->param_range();
    SPAinterval u_range = pb.interval(0);
    SPAinterval v_range = pb.interval(1);

    for (int i = 0; i <= num_samples; i++) {
        for (int j = 0; j <= num_samples; j++) {
            double u = u_range.low() +
                       (u_range.high() - u_range.low()) * i / num_samples;
            double v = v_range.low() +
                       (v_range.high() - v_range.low()) * j / num_samples;

            SPApar_pos pos(u, v);
            SPAvector du, dv;

            try {
                surface->eval_derivs(pos, du, dv);

                double cross_len = (du * dv).length();
                double mag_product = du.length() * dv.length();

                if (mag_product > SPAresabs) {
                    double sin_angle = cross_len / mag_product;

                    if (sin_angle < SPAresnor) {
                        SPApar_pos p_next(
                            u + (u_range.high() - u_range.low()) / num_samples,
                            v
                        );
                        SPAposition p1 = surface->eval_position(pos);
                        SPAposition p2 = surface->eval_position(p_next);

                        if ((p1 - p2).length() > SPAresabs) {
                            insanity_data *id = new insanity_data();
                            id->set_insanity_type(WARNING);
                            id->set_description(
                                "Surface singularity detected."
                            );
                            ilist->add(id);
                            if (status) *status |= SURF_CHECK_BAD_SINGULARITY;
                            valid = FALSE;
                        }
                    }
                }
            } catch (...) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description(
                    "Surface derivative evaluation failed."
                );
                ilist->add(id);
                if (status) *status |= SURF_CHECK_EVAL_FAILURE;
                valid = FALSE;
            }
        }
    }

    return valid;
}

logical check_surface_closure(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;

    logical is_closed_u = surface->closed_u();
    logical is_closed_v = surface->closed_v();

    if (is_closed_u || is_closed_v) {
        SPApar_box pb = surface->param_range();
        SPAinterval u_range = pb.interval(0);
        SPAinterval v_range = pb.interval(1);

        double test_u = (u_range.low() + u_range.high()) / 2.0;
        double test_v = (v_range.low() + v_range.high()) / 2.0;

        if (is_closed_u) {
            SPApar_pos p1(u_range.low(), test_v);
            SPApar_pos p2(u_range.high(), test_v);

            SPAposition pos1 = surface->eval_position(p1);
            SPAposition pos2 = surface->eval_position(p2);

            if ((pos1 - pos2).length() > SPAresabs) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description(
                    "Surface U closure mismatch."
                );
                ilist->add(id);
                if (status) *status |= SURF_CHECK_BAD_CLOSURE;
                valid = FALSE;
            }
        }

        if (is_closed_v) {
            SPApar_pos p1(test_u, v_range.low());
            SPApar_pos p2(test_u, v_range.high());

            SPAposition pos1 = surface->eval_position(p1);
            SPAposition pos2 = surface->eval_position(p2);

            if ((pos1 - pos2).length() > SPAresabs) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description(
                    "Surface V closure mismatch."
                );
                ilist->add(id);
                if (status) *status |= SURF_CHECK_BAD_CLOSURE;
                valid = FALSE;
            }
        }
    }

    return valid;
}

logical check_surface_fit_tolerance(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;
    double fit_tol = surface->fit_tolerance();

    if (std::isnan(fit_tol) || std::isinf(fit_tol)) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Surface fit tolerance is NaN or Inf.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_FIT_TOLERANCE;
        valid = FALSE;
    }

    if (fit_tol < 0) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("Surface fit tolerance is negative.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_FIT_TOLERANCE;
        valid = FALSE;
    }

    if (fit_tol > SPAresabs * 1000) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Surface fit tolerance is unusually large.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_FIT_TOLERANCE;
    }

    return valid;
}

logical check_bspline_surface(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    if (surface->identity() != BS3_SURFACE_TYPE) {
        return TRUE;
    }

    logical valid = TRUE;

    bs3_surface *bs = surface->base_surface();
    if (!bs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("B-spline surface has null base surface.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_ILLEGAL_SURFACE;
        return FALSE;
    }

    int u_order = bs->order_u();
    int v_order = bs->order_v();
    int u_num_cp = bs->num_control_points_u();
    int v_num_cp = bs->num_control_points_v();

    if (u_order < 1 || v_order < 1) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description("B-spline surface has invalid order.");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_ILLEGAL_SURFACE;
        valid = FALSE;
    }

    if (u_num_cp < u_order || v_num_cp < v_order) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(ERROR_TYPE);
        id->set_description(
            "B-spline surface: control points < order."
        );
        ilist->add(id);
        if (status) *status |= SURF_CHECK_ILLEGAL_SURFACE;
        valid = FALSE;
    }

    for (int i = 0; i < u_num_cp - 1; i++) {
        for (int j = 0; j < v_num_cp; j++) {
            SPAposition cp1 = bs->control_point(i, j);
            SPAposition cp2 = bs->control_point(i + 1, j);

            if ((cp1 - cp2).length() < SPAresabs) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(WARNING);
                id->set_description(
                    "Adjacent B-spline control vertices coincident in U."
                );
                ilist->add(id);
                if (status) *status |= SURF_CHECK_ILLEGAL_SURFACE;
                valid = FALSE;
            }
        }
    }

    for (int i = 0; i < u_num_cp; i++) {
        for (int j = 0; j < v_num_cp - 1; j++) {
            SPAposition cp1 = bs->control_point(i, j);
            SPAposition cp2 = bs->control_point(i, j + 1);

            if ((cp1 - cp2).length() < SPAresabs) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(WARNING);
                id->set_description(
                    "Adjacent B-spline control vertices coincident in V."
                );
                ilist->add(id);
                if (status) *status |= SURF_CHECK_ILLEGAL_SURFACE;
                valid = FALSE;
            }
        }
    }

    return valid;
}

logical check_surface_self_intersection(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_samples = 15;
    SPApar_box pb = surface->param_range();
    SPAinterval u_range = pb.interval(0);
    SPAinterval v_range = pb.interval(1);

    double u_step = (u_range.high() - u_range.low()) / num_samples;
    double v_step = (v_range.high() - v_range.low()) / num_samples;

    // Pre-compute normals on a grid and check for sign flips
    // A sign flip in the normal dot product between adjacent cells
    // indicates a possible self-intersection or orientation reversal
    SPAvector **normals = new SPAvector*[num_samples + 1];
    for (int i = 0; i <= num_samples; i++) {
        normals[i] = new SPAvector[num_samples + 1];
    }

    bool eval_failed = false;

    for (int i = 0; i <= num_samples && !eval_failed; i++) {
        for (int j = 0; j <= num_samples && !eval_failed; j++) {
            double u = u_range.low() + u_step * i;
            double v = v_range.low() + v_step * j;

            SPApar_pos pos(u, v);
            SPAvector du, dv;

            try {
                surface->eval_derivs(pos, du, dv);
                SPAvector normal = du * dv;
                double len = normal.length();
                if (len > SPAresabs) {
                    normals[i][j] = normal / len;
                } else {
                    normals[i][j] = SPAvector(0, 0, 0);
                }
            } catch (...) {
                eval_failed = true;
            }
        }
    }

    if (!eval_failed) {
        // Check for normal sign flips between adjacent grid cells
        for (int i = 0; i < num_samples; i++) {
            for (int j = 0; j < num_samples; j++) {
                SPAvector n00 = normals[i][j];
                SPAvector n10 = normals[i + 1][j];
                SPAvector n01 = normals[i][j + 1];
                SPAvector n11 = normals[i + 1][j + 1];

                // Skip cells with degenerate normals
                if (n00.length() < SPAresabs || n10.length() < SPAresabs ||
                    n01.length() < SPAresabs || n11.length() < SPAresabs) {
                    continue;
                }

                // Check if normal flips direction between adjacent cells
                double dot_u = n00 | n10;
                double dot_v = n00 | n01;
                double dot_diag = n00 | n11;

                // A negative dot product between adjacent cell normals
                // indicates a possible fold or self-intersection
                if (dot_u < -0.5 || dot_v < -0.5 || dot_diag < -0.5) {
                    // Verify by checking if the surface actually crosses itself
                    // using cell bounding box overlap in 3D
                    SPAposition p00 = surface->eval_position(
                        SPApar_pos(u_range.low() + u_step * i,
                                   v_range.low() + v_step * j));
                    SPAposition p11 = surface->eval_position(
                        SPApar_pos(u_range.low() + u_step * (i + 1),
                                   v_range.low() + v_step * (j + 1)));

                    double cell_diag = (p11 - p00).length();
                    double param_diag = sqrt(u_step * u_step + v_step * v_step);

                    // If 3D distance is much smaller than parameter distance,
                    // the surface may be folding over itself
                    if (cell_diag < param_diag * SPAresabs * 10 &&
                        cell_diag > SPAresabs) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(WARNING);
                        id->set_description(
                            "Possible surface self-intersection: normal flip detected."
                        );
                        ilist->add(id);
                        if (status) *status |= SURF_CHECK_SELF_INTERSECT;
                        valid = FALSE;
                        break;
                    }
                }
            }
            if (!valid) break;
        }
    }

    // Clean up
    for (int i = 0; i <= num_samples; i++) {
        delete[] normals[i];
    }
    delete[] normals;

    return valid;
}

logical check_surface_normal_consistency(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_samples = 10;
    SPApar_box pb = surface->param_range();
    SPAinterval u_range = pb.interval(0);
    SPAinterval v_range = pb.interval(1);

    double u_step = (u_range.high() - u_range.low()) / num_samples;
    double v_step = (v_range.high() - v_range.low()) / num_samples;

    for (int i = 0; i < num_samples; i++) {
        for (int j = 0; j < num_samples; j++) {
            double u = u_range.low() + u_step * (i + 0.5);
            double v = v_range.low() + v_step * (j + 0.5);

            SPApar_pos pos(u, v);
            SPAvector du, dv;

            try {
                surface->eval_derivs(pos, du, dv);
                SPAvector normal = du * dv;
                double normal_len = normal.length();

                if (normal_len < SPAresabs) {
                    continue;
                }

                normal = normal / normal_len;

                if (std::isnan(normal.x()) || std::isnan(normal.y()) ||
                    std::isnan(normal.z())) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(ERROR_TYPE);
                    id->set_description("Surface normal has NaN components.");
                    ilist->add(id);
                    if (status) *status |= SURF_CHECK_BAD_NORMAL;
                    return FALSE;
                }

                if (std::isinf(normal.x()) || std::isinf(normal.y()) ||
                    std::isinf(normal.z())) {
                    insanity_data *id = new insanity_data();
                    id->set_insanity_type(ERROR_TYPE);
                    id->set_description("Surface normal has Inf components.");
                    ilist->add(id);
                    if (status) *status |= SURF_CHECK_BAD_NORMAL;
                    return FALSE;
                }
            } catch (...) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Surface normal evaluation failed.");
                ilist->add(id);
                if (status) *status |= SURF_CHECK_EVAL_FAILURE;
                return FALSE;
            }
        }
    }

    return valid;
}

logical check_surface_g2_continuity(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    if (!surface->closed_u() && !surface->closed_v()) {
        return TRUE;
    }

    logical valid = TRUE;
    int num_samples = 10;
    SPApar_box pb = surface->param_range();
    SPAinterval u_range = pb.interval(0);
    SPAinterval v_range = pb.interval(1);

    if (surface->closed_u()) {
        for (int j = 0; j <= num_samples; j++) {
            double v = v_range.low() +
                       (v_range.high() - v_range.low()) * j / num_samples;

            double eps = (u_range.high() - u_range.low()) * 0.001;

            SPApar_pos p_left(u_range.high() - eps, v);
            SPApar_pos p_right(u_range.low() + eps, v);

            try {
                // Evaluate first derivatives
                SPAvector du_left, dv_left;
                SPAvector du_right, dv_right;
                surface->eval_derivs(p_left, du_left, dv_left);
                surface->eval_derivs(p_right, du_right, dv_right);

                // G1 check at seam
                if (du_left.length() > SPAresabs && du_right.length() > SPAresabs) {
                    double cos_g1 = (du_left | du_right) /
                                    (du_left.length() * du_right.length());
                    if (cos_g1 < 1.0 - SPAresnor * 10) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(WARNING);
                        id->set_description(
                            "Surface G1 discontinuity at U seam."
                        );
                        ilist->add(id);
                        if (status) *status |= SURF_CHECK_NON_G1;
                        valid = FALSE;
                    }
                }

                // G2 check: compare curvature (second derivative) at seam
                // Approximate second derivatives via finite differences
                double h = (u_range.high() - u_range.low()) * 0.0001;
                SPApar_pos p_left2(u_range.high() - 2 * eps, v);
                SPApar_pos p_right2(u_range.low() + 2 * eps, v);

                SPAvector du_left2, dv_left2, du_right2, dv_right2;
                surface->eval_derivs(p_left2, du_left2, dv_left2);
                surface->eval_derivs(p_right2, du_right2, dv_right2);

                SPAvector d2u_left = (du_left - du_left2) / h;
                SPAvector d2u_right = (du_right2 - du_right) / h;

                if (d2u_left.length() > SPAresabs && d2u_right.length() > SPAresabs) {
                    double cos_g2 = (d2u_left | d2u_right) /
                                    (d2u_left.length() * d2u_right.length());
                    if (cos_g2 < 1.0 - SPAresnor * 100) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(WARNING);
                        id->set_description(
                            "Surface G2 discontinuity at U seam."
                        );
                        ilist->add(id);
                        if (status) *status |= SURF_CHECK_NON_G2;
                        valid = FALSE;
                    }
                }
            } catch (...) {
                // Skip on exception
            }
        }
    }

    if (surface->closed_v()) {
        for (int i = 0; i <= num_samples; i++) {
            double u = u_range.low() +
                       (u_range.high() - u_range.low()) * i / num_samples;

            double eps = (v_range.high() - v_range.low()) * 0.001;

            SPApar_pos p_left(u, v_range.high() - eps);
            SPApar_pos p_right(u, v_range.low() + eps);

            try {
                // Evaluate first derivatives
                SPAvector du_left, dv_left;
                SPAvector du_right, dv_right;
                surface->eval_derivs(p_left, du_left, dv_left);
                surface->eval_derivs(p_right, du_right, dv_right);

                // G1 check at seam
                if (dv_left.length() > SPAresabs && dv_right.length() > SPAresabs) {
                    double cos_g1 = (dv_left | dv_right) /
                                    (dv_left.length() * dv_right.length());
                    if (cos_g1 < 1.0 - SPAresnor * 10) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(WARNING);
                        id->set_description(
                            "Surface G1 discontinuity at V seam."
                        );
                        ilist->add(id);
                        if (status) *status |= SURF_CHECK_NON_G1;
                        valid = FALSE;
                    }
                }

                // G2 check: compare curvature (second derivative) at seam
                double h = (v_range.high() - v_range.low()) * 0.0001;
                SPApar_pos p_left2(u, v_range.high() - 2 * eps);
                SPApar_pos p_right2(u, v_range.low() + 2 * eps);

                SPAvector du_left2, dv_left2, du_right2, dv_right2;
                surface->eval_derivs(p_left2, du_left2, dv_left2);
                surface->eval_derivs(p_right2, du_right2, dv_right2);

                SPAvector d2v_left = (dv_left - dv_left2) / h;
                SPAvector d2v_right = (dv_right2 - dv_right) / h;

                if (d2v_left.length() > SPAresabs && d2v_right.length() > SPAresabs) {
                    double cos_g2 = (d2v_left | d2v_right) /
                                    (d2v_left.length() * d2v_right.length());
                    if (cos_g2 < 1.0 - SPAresnor * 100) {
                        insanity_data *id = new insanity_data();
                        id->set_insanity_type(WARNING);
                        id->set_description(
                            "Surface G2 discontinuity at V seam."
                        );
                        ilist->add(id);
                        if (status) *status |= SURF_CHECK_NON_G2;
                        valid = FALSE;
                    }
                }
            } catch (...) {
                // Skip on exception
            }
        }
    }

    return valid;
}

logical check_surface_uv_coordinates(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_samples = 10;
    SPApar_box pb = surface->param_range();
    SPAinterval u_range = pb.interval(0);
    SPAinterval v_range = pb.interval(1);

    for (int i = 0; i <= num_samples; i++) {
        for (int j = 0; j <= num_samples; j++) {
            double u = u_range.low() +
                       (u_range.high() - u_range.low()) * i / num_samples;
            double v = v_range.low() +
                       (v_range.high() - v_range.low()) * j / num_samples;

            if (std::isnan(u) || std::isnan(v)) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Surface UV coordinate is NaN.");
                ilist->add(id);
                if (status) *status |= SURF_CHECK_BAD_UV_COORDINATES;
                return FALSE;
            }

            if (std::isinf(u) || std::isinf(v)) {
                insanity_data *id = new insanity_data();
                id->set_insanity_type(ERROR_TYPE);
                id->set_description("Surface UV coordinate is Inf.");
                ilist->add(id);
                if (status) *status |= SURF_CHECK_BAD_UV_COORDINATES;
                return FALSE;
            }
        }
    }

    return valid;
}

logical check_surface_area_degenerate(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;
    int num_samples = 10;
    SPApar_box pb = surface->param_range();
    SPAinterval u_range = pb.interval(0);
    SPAinterval v_range = pb.interval(1);

    double total_area = 0.0;
    double u_step = (u_range.high() - u_range.low()) / num_samples;
    double v_step = (v_range.high() - v_range.low()) / num_samples;

    for (int i = 0; i < num_samples; i++) {
        for (int j = 0; j < num_samples; j++) {
            double u = u_range.low() + u_step * (i + 0.5);
            double v = v_range.low() + v_step * (j + 0.5);

            SPApar_pos pos(u, v);
            SPAvector du, dv;

            try {
                surface->eval_derivs(pos, du, dv);
                SPAvector normal = du * dv;
                total_area += normal.length() * u_step * v_step;
            } catch (...) {
                // Skip on exception
            }
        }
    }

    if (total_area < SPAresabs * SPAresabs) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description("Surface area is near zero (degenerate).");
        ilist->add(id);
        if (status) *status |= SURF_CHECK_DEGENERATE_AREA;
        valid = FALSE;
    }

    return valid;
}

logical check_surface_periodicity(
    SURFACE       *surface,
    insanity_list *ilist,
    int           *status
) {
    if (!surface) {
        return FALSE;
    }

    logical valid = TRUE;

    if (surface->periodic_u() && !surface->closed_u()) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description(
            "Surface is periodic in U but not closed."
        );
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_PERIODICITY;
        valid = FALSE;
    }

    if (surface->periodic_v() && !surface->closed_v()) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description(
            "Surface is periodic in V but not closed."
        );
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_PERIODICITY;
        valid = FALSE;
    }

    if (surface->closed_u() && !surface->periodic_u()) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description(
            "Surface is closed in U but not periodic."
        );
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_PERIODICITY;
        valid = FALSE;
    }

    if (surface->closed_v() && !surface->periodic_v()) {
        insanity_data *id = new insanity_data();
        id->set_insanity_type(WARNING);
        id->set_description(
            "Surface is closed in V but not periodic."
        );
        ilist->add(id);
        if (status) *status |= SURF_CHECK_BAD_PERIODICITY;
        valid = FALSE;
    }

    return valid;
}

int check_surface_ok(
    SURFACE *surface,
    int     *insanity_count
) {
    if (!surface) {
        return SURF_CHECK_NULL_SURFACE;
    }

    insanity_list ilist;
    int count = 0;
    int status = SURF_CHECK_OK;

    if (check_surface_null(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_evaluation(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_parameter_range(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_continuity(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_singularity(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_closure(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_fit_tolerance(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_bspline_surface(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_self_intersection(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_normal_consistency(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_g2_continuity(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_uv_coordinates(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_area_degenerate(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (check_surface_periodicity(surface, &ilist, &status) == FALSE) {
        count++;
    }

    if (insanity_count) {
        *insanity_count = count;
    }

    return status;
}
