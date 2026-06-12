# ACIS 几何检查接口实现总结

## 项目概述

基于 ACIS 3D 内核文档，实现了 5 个几何实体的检查验证模块，共 74 个检查函数（64 个子检查函数 + 10 个主函数），覆盖拓扑、几何、数值有效性等维度。

---

## 文件清单

### 头文件 (`include/`)

| 文件 | 行数 | 说明 |
|------|------|------|
| `check_lump.hxx` | 116 | LUMP 检查接口声明 |
| `check_vertex.hxx` | 109 | VERTEX 检查接口声明 |
| `check_edge.hxx` | 128 | EDGE 检查接口声明 |
| `check_surface.hxx` | 132 | SURFACE 检查接口声明 |
| `bs3_curve_check.hxx` | 136 | BS3_CURVE 检查接口声明 |

### 实现文件 (`src/`)

| 文件 | 行数 | 说明 |
|------|------|------|
| `check_lump.cxx` | 765 | LUMP 检查实现 |
| `check_vertex.cxx` | 655 | VERTEX 检查实现 |
| `check_edge.cxx` | 819 | EDGE 检查实现 |
| `check_surface.cxx` | 1002 | SURFACE 检查实现 |
| `bs3_curve_check.cxx` | 938 | BS3_CURVE 检查实现 |

---

## 接口列表

### 1. LUMP 检查模块

#### 枚举 `lump_check_status`

| 枚举值 | 值 | 说明 |
|--------|-----|------|
| `LUMP_CHECK_OK` | 0 | 无错误 |
| `LUMP_CHECK_NO_SHELL` | 1<<0 | 无 Shell |
| `LUMP_CHECK_EMPTY_SHELL` | 1<<1 | 空 Shell |
| `LUMP_CHECK_SHELL_SELF_INT` | 1<<2 | Shell 自交 |
| `LUMP_CHECK_BAD_CONTAINMENT` | 1<<3 | 包含关系错误 |
| `LUMP_CHECK_INTERSECT_SHELLS` | 1<<4 | Shell 相交 |
| `LUMP_CHECK_DEGENERATE_FACE` | 1<<5 | 退化面 |
| `LUMP_CHECK_BAD_COEDGE_SENSE` | 1<<6 | Coedge 方向错误 |
| `LUMP_CHECK_NULL_EDGE_CURVE` | 1<<7 | 边曲线为空 |
| `LUMP_CHECK_NON_MANIFOLD_VTX` | 1<<8 | 非流形顶点 |
| `LUMP_CHECK_BAD_VOLUME` | 1<<9 | 体积异常 |
| `LUMP_CHECK_BAD_BOUNDING_BOX` | 1<<10 | 包围盒异常 |
| `LUMP_CHECK_SHELL_ORIENT_MISMATCH` | 1<<11 | Shell 方向不一致 |
| `LUMP_CHECK_BAD_FACE_ADJACENCY` | 1<<12 | 面邻接异常 |
| `LUMP_CHECK_NON_MANIFOLD_EDGE` | 1<<13 | 非流形边 |

#### 函数

| 函数 | 返回类型 | 说明 |
|------|----------|------|
| `api_check_lump(LUMP*, lump_check_result&, AcisOptions*)` | `outcome` | 详细诊断 |
| `api_check_lump_status(LUMP*, int*)` | `int` | 快速检测 |
| `check_lump_shells_valid(LUMP*, insanity_list*)` | `logical` | Shell 有效性 |
| `check_shell_faces_valid(SHELL*, insanity_list*)` | `logical` | 面有效性 |
| `check_lump_containment(LUMP*, insanity_list*)` | `logical` | 包含关系 |
| `check_edge_curves_valid(FACE*, insanity_list*)` | `logical` | 边曲线有效性 |
| `check_coedge_sense(FACE*, insanity_list*)` | `logical` | Coedge 方向 |
| `check_wire_self_intersect(WIRE*, insanity_list*)` | `logical` | Wire 自交 |
| `check_lump_volume(LUMP*, insanity_list*)` | `logical` | 体积检查 |
| `check_lump_bounding_box(LUMP*, insanity_list*)` | `logical` | 包围盒检查 |
| `check_lump_shell_orientation(LUMP*, insanity_list*)` | `logical` | Shell 方向 |
| `check_lump_face_adjacency(LUMP*, insanity_list*)` | `logical` | 面邻接检查 |
| `check_lump_edge_manifold(LUMP*, insanity_list*)` | `logical` | 边流形检查 |

---

### 2. VERTEX 检查模块

#### 枚举 `vertex_check_status`

| 枚举值 | 值 | 说明 |
|--------|-----|------|
| `VTX_CHECK_OK` | 0 | 无错误 |
| `VTX_CHECK_NULL_POINT` | 1<<0 | POINT 为空 |
| `VTX_CHECK_NO_EDGES` | 1<<1 | 无关联边 |
| `VTX_CHECK_DEGENERATE_EDGE` | 1<<2 | 退化边 |
| `VTX_CHECK_BAD_EDGE_CURVE` | 1<<3 | 边曲线无效 |
| `VTX_CHECK_EDGE_SENSE_MISMATCH` | 1<<4 | 方向不匹配 |
| `VTX_CHECK_NON_MANIFOLD` | 1<<5 | 非流形 |
| `VTX_CHECK_COINCIDENT_VERTICES` | 1<<6 | 共点顶点 |
| `VTX_CHECK_POINT_NOT_ON_CURVE` | 1<<7 | 顶点不在曲线上 |
| `VTX_CHECK_BAD_BOUNDING_BOX` | 1<<8 | 包围盒异常 |
| `VTX_CHECK_BAD_NORMAL_CONSISTENCY` | 1<<9 | 法向不一致 |
| `VTX_CHECK_BAD_TOLERANCE` | 1<<10 | 容差异常 |
| `VTX_CHECK_SHARP_ANGLE` | 1<<11 | 尖角 |

#### 函数

| 函数 | 返回类型 | 说明 |
|------|----------|------|
| `api_check_vertex(VERTEX*, int*)` | `int` | 快速检测 |
| `api_check_vertex_errors(VERTEX*, vertex_check_result&, AcisOptions*)` | `outcome` | 详细诊断 |
| `check_vertex_point_valid(VERTEX*, insanity_list*)` | `logical` | POINT 有效性 |
| `check_vertex_edges_valid(VERTEX*, insanity_list*)` | `logical` | 边有效性 |
| `check_vertex_edge_curves(VERTEX*, insanity_list*)` | `logical` | 边曲线检查 |
| `check_vertex_coincident(VERTEX*, insanity_list*)` | `logical` | 共点检查 |
| `check_vertex_edge_sense(VERTEX*, insanity_list*)` | `logical` | 方向一致性 |
| `check_vertex_manifold(VERTEX*, insanity_list*)` | `logical` | 流形检查 |
| `check_vertex_bounding_box(VERTEX*, insanity_list*)` | `logical` | 包围盒检查 |
| `check_vertex_normal_consistency(VERTEX*, insanity_list*)` | `logical` | 法向一致性 |
| `check_vertex_tolerance(VERTEX*, insanity_list*)` | `logical` | 容差检查 |
| `check_vertex_sharp_angle(VERTEX*, insanity_list*)` | `logical` | 尖角检查 |

---

### 3. EDGE 检查模块

#### 枚举 `edge_check_status`

| 枚举值 | 值 | 说明 |
|--------|-----|------|
| `EDGE_CHECK_OK` | 0 | 无错误 |
| `EDGE_CHECK_NULL_EDGE` | 1<<0 | 边为空 |
| `EDGE_CHECK_NULL_CURVE` | 1<<1 | 曲线为空 |
| `EDGE_CHECK_NULL_VERTEX` | 1<<2 | 顶点为空 |
| `EDGE_CHECK_DEGENERATE` | 1<<3 | 退化边 |
| `EDGE_CHECK_BAD_PARAM_RANGE` | 1<<4 | 参数域异常 |
| `EDGE_CHECK_VERTEX_NOT_ON_CURVE` | 1<<5 | 顶点不在曲线上 |
| `EDGE_CHECK_BAD_CLOSURE` | 1<<6 | 闭合异常 |
| `EDGE_CHECK_COEDGE_SENSE_ERROR` | 1<<7 | Coedge 方向错误 |
| `EDGE_CHECK_EVAL_FAILURE` | 1<<8 | 评估失败 |
| `EDGE_CHECK_NAN_COORDINATES` | 1<<9 | NaN/Inf |
| `EDGE_CHECK_BAD_FIT_TOLERANCE` | 1<<10 | 拟合公差异常 |
| `EDGE_CHECK_BAD_LENGTH` | 1<<11 | 长度异常 |
| `EDGE_CHECK_NON_G1_CONTINUITY` | 1<<12 | G1 连续性 |
| `EDGE_CHECK_BAD_BOUNDING_BOX` | 1<<13 | 包围盒异常 |
| `EDGE_CHECK_BAD_PARAM_NORMALIZATION` | 1<<14 | 参数归一化 |

#### 函数

| 函数 | 返回类型 | 说明 |
|------|----------|------|
| `api_check_edge(EDGE*, int*)` | `int` | 快速检测 |
| `api_check_edge_errors(EDGE*, edge_check_result&, AcisOptions*)` | `outcome` | 详细诊断 |
| `check_edge_null(EDGE*, insanity_list*)` | `logical` | 空指针检查 |
| `check_edge_curve(EDGE*, insanity_list*)` | `logical` | 曲线有效性 |
| `check_edge_vertices(EDGE*, insanity_list*)` | `logical` | 顶点有效性 |
| `check_edge_degenerate(EDGE*, insanity_list*)` | `logical` | 退化检查 |
| `check_edge_parameter_range(EDGE*, insanity_list*)` | `logical` | 参数域检查 |
| `check_edge_vertex_on_curve(EDGE*, insanity_list*)` | `logical` | 顶点在曲线上 |
| `check_edge_closure(EDGE*, insanity_list*)` | `logical` | 闭合检查 |
| `check_edge_coedge_sense(EDGE*, insanity_list*)` | `logical` | 方向一致性 |
| `check_edge_evaluation(EDGE*, insanity_list*)` | `logical` | 评估检查 |
| `check_edge_fit_tolerance(EDGE*, insanity_list*)` | `logical` | 拟合公差 |
| `check_edge_length(EDGE*, insanity_list*)` | `logical` | 长度检查 |
| `check_edge_g1_continuity(EDGE*, insanity_list*)` | `logical` | G1 连续性 |
| `check_edge_bounding_box(EDGE*, insanity_list*)` | `logical` | 包围盒检查 |
| `check_edge_param_normalization(EDGE*, insanity_list*)` | `logical` | 参数归一化 |

---

### 4. SURFACE 检查模块

#### 枚举 `surface_check_status`

| 枚举值 | 值 | 说明 |
|--------|-----|------|
| `SURF_CHECK_OK` | 0 | 无错误 |
| `SURF_CHECK_NULL_SURFACE` | 1<<0 | 曲面为空 |
| `SURF_CHECK_EVAL_FAILURE` | 1<<1 | 评估失败 |
| `SURF_CHECK_NAN_COORDINATES` | 1<<2 | NaN/Inf |
| `SURF_CHECK_BAD_PARAMETER_RANGE` | 1<<3 | 参数域异常 |
| `SURF_CHECK_SELF_INTERSECT` | 1<<4 | 自交 |
| `SURF_CHECK_BAD_CLOSURE` | 1<<5 | 闭合异常 |
| `SURF_CHECK_NON_G0` | 1<<6 | G0 连续性 |
| `SURF_CHECK_NON_G1` | 1<<7 | G1 连续性 |
| `SURF_CHECK_BAD_FIT_TOLERANCE` | 1<<8 | 拟合公差异常 |
| `SURF_CHECK_BAD_SINGULARITY` | 1<<9 | 奇异点 |
| `SURF_CHECK_ILLEGAL_SURFACE` | 1<<10 | 非法曲面 |
| `SURF_CHECK_BAD_NORMAL` | 1<<11 | 法向异常 |
| `SURF_CHECK_NON_G2` | 1<<12 | G2 连续性 |
| `SURF_CHECK_BAD_UV_COORDINATES` | 1<<13 | UV 坐标异常 |
| `SURF_CHECK_DEGENERATE_AREA` | 1<<14 | 面积退化 |
| `SURF_CHECK_BAD_PERIODICITY` | 1<<15 | 周期性异常 |

#### 函数

| 函数 | 返回类型 | 说明 |
|------|----------|------|
| `check_surface_ok(SURFACE*, int*)` | `int` | 快速检测 |
| `api_check_surface_ok(SURFACE*, surface_check_result&, AcisOptions*)` | `outcome` | 详细诊断 |
| `check_surface_null(SURFACE*, insanity_list*)` | `logical` | 空指针检查 |
| `check_surface_evaluation(SURFACE*, insanity_list*)` | `logical` | 评估检查 |
| `check_surface_parameter_range(SURFACE*, insanity_list*)` | `logical` | 参数域检查 |
| `check_surface_continuity(SURFACE*, insanity_list*)` | `logical` | 闭合连续性 |
| `check_surface_singularity(SURFACE*, insanity_list*)` | `logical` | 奇异点检查 |
| `check_surface_closure(SURFACE*, insanity_list*)` | `logical` | 闭合检查 |
| `check_surface_fit_tolerance(SURFACE*, insanity_list*)` | `logical` | 拟合公差 |
| `check_bspline_surface(SURFACE*, insanity_list*)` | `logical` | B-spline 检查 |
| `check_surface_self_intersection(SURFACE*, insanity_list*)` | `logical` | 自交检查 |
| `check_surface_normal_consistency(SURFACE*, insanity_list*)` | `logical` | 法向一致性 |
| `check_surface_g2_continuity(SURFACE*, insanity_list*)` | `logical` | G2 连续性 |
| `check_surface_uv_coordinates(SURFACE*, insanity_list*)` | `logical` | UV 坐标 |
| `check_surface_area_degenerate(SURFACE*, insanity_list*)` | `logical` | 面积退化 |
| `check_surface_periodicity(SURFACE*, insanity_list*)` | `logical` | 周期性检查 |

---

### 5. BS3_CURVE 检查模块

#### 枚举 `bs3_curve_check_status`

| 枚举值 | 值 | 说明 |
|--------|-----|------|
| `BS3_CURVE_CHECK_OK` | 0 | 无错误 |
| `BS3_CURVE_CHECK_NULL_CURVE` | 1<<0 | 曲线为空 |
| `BS3_CURVE_CHECK_BAD_ORDER` | 1<<1 | 阶数异常 |
| `BS3_CURVE_CHECK_BAD_KNOT_VECTOR` | 1<<2 | 节点向量异常 |
| `BS3_CURVE_CHECK_BAD_CP_COUNT` | 1<<3 | 控制点数量异常 |
| `BS3_CURVE_CHECK_COINCIDENT_CPS` | 1<<4 | 控制点重合 |
| `BS3_CURVE_CHECK_EVAL_FAILURE` | 1<<5 | 评估失败 |
| `BS3_CURVE_CHECK_NAN_COORDINATES` | 1<<6 | NaN/Inf |
| `BS3_CURVE_CHECK_BAD_PARAM_RANGE` | 1<<7 | 参数域异常 |
| `BS3_CURVE_CHECK_BAD_CLOSURE` | 1<<8 | 闭合异常 |
| `BS3_CURVE_CHECK_BAD_FIT_TOL` | 1<<9 | 拟合公差异常 |
| `BS3_CURVE_CHECK_BAD_KNOT_MULT` | 1<<10 | 节点重数异常 |
| `BS3_CURVE_CHECK_DEGENERATE` | 1<<11 | 退化 |
| `BS3_CURVE_CHECK_BAD_CONVEX_HULL` | 1<<12 | 凸包性质 |
| `BS3_CURVE_CHECK_BAD_VD_PROPERTY` | 1<<13 | 变差缩减性质 |
| `BS3_CURVE_CHECK_BAD_BOUNDING_BOX` | 1<<14 | 包围盒异常 |
| `BS3_CURVE_CHECK_BAD_ARC_LENGTH` | 1<<15 | 弧长异常 |

#### 函数

| 函数 | 返回类型 | 说明 |
|------|----------|------|
| `bs3_curve_check(BS3_CURVE*, int*)` | `int` | 快速检测 |
| `api_bs3_curve_check(BS3_CURVE*, bs3_curve_check_result&, AcisOptions*)` | `outcome` | 详细诊断 |
| `check_bs3_curve_null(BS3_CURVE*, insanity_list*)` | `logical` | 空指针检查 |
| `check_bs3_curve_order(BS3_CURVE*, insanity_list*)` | `logical` | 阶数检查 |
| `check_bs3_curve_control_points(BS3_CURVE*, insanity_list*)` | `logical` | 控制点检查 |
| `check_bs3_curve_knot_vector(BS3_CURVE*, insanity_list*)` | `logical` | 节点向量检查 |
| `check_bs3_curve_evaluation(BS3_CURVE*, insanity_list*)` | `logical` | 评估检查 |
| `check_bs3_curve_parameter_range(BS3_CURVE*, insanity_list*)` | `logical` | 参数域检查 |
| `check_bs3_curve_closure(BS3_CURVE*, insanity_list*)` | `logical` | 闭合检查 |
| `check_bs3_curve_fit_tolerance(BS3_CURVE*, insanity_list*)` | `logical` | 拟合公差 |
| `check_bs3_curve_degeneracy(BS3_CURVE*, insanity_list*)` | `logical` | 退化检查 |
| `check_bs3_curve_derivatives(BS3_CURVE*, insanity_list*)` | `logical` | 导数检查 |
| `check_bs3_curve_knot_multiplicity(BS3_CURVE*, insanity_list*)` | `logical` | 节点重数 |
| `check_bs3_curve_convex_hull(BS3_CURVE*, insanity_list*)` | `logical` | 凸包性质 |
| `check_bs3_curve_vd_property(BS3_CURVE*, insanity_list*)` | `logical` | 变差缩减 |
| `check_bs3_curve_bounding_box(BS3_CURVE*, insanity_list*)` | `logical` | 包围盒检查 |
| `check_bs3_curve_arc_length(BS3_CURVE*, insanity_list*)` | `logical` | 弧长检查 |

---

## 接口模式

每个模块提供两种调用方式：

### 1. 快速检测 (int 返回)

```c
int status = api_check_vertex(vtx, &count);
if (status == VTX_CHECK_OK) { /* 有效 */ }
if (status & VTX_CHECK_NON_MANIFOLD) { /* 非流形 */ }
```

### 2. 详细诊断 (outcome + result)

```c
vertex_check_result result;
outcome res = api_check_vertex_errors(vtx, result);
if (!result.is_ok()) {
    insanity_list *ilist = result.get_insanity_list();
    // 遍历 ilist 获取详细错误
}
```

---

## 依赖的 ACIS 接口

| 类/函数 | 头文件 | 用途 |
|---------|--------|------|
| `LUMP`, `SHELL`, `FACE` | `lump.hxx`, `shell.hxx`, `face.hxx` | 拓扑遍历 |
| `EDGE`, `VERTEX`, `COEDGE` | `edge.hxx`, `vertex.hxx`, `coedge.hxx` | 拓扑遍历 |
| `CURVE`, `SURFACE`, `POINT` | `curve.hxx`, `surface.hxx`, `point.hxx` | 几何评估 |
| `BS3_CURVE`, `bs3_surface` | `bs3_curve.hxx`, `bs3_surface.hxx` | B-spline 检查 |
| `SPAposition`, `SPAvector`, `SPAinterval` | `SPAposition.hxx`, `SPAvector.hxx` | 数学运算 |
| `insanity_list`, `insanity_data` | `insanity_list.hxx` | 错误报告 |
| `SPAresabs`, `SPAresnor` | `SPAres.hxx` | 容差常量 |

---

## 统计

| 模块 | 枚举项 | 子检查函数 | 主函数 |
|------|--------|-----------|--------|
| LUMP | 14 | 11 | 2 |
| VERTEX | 12 | 10 | 2 |
| EDGE | 15 | 14 | 2 |
| SURFACE | 16 | 14 | 2 |
| BS3_CURVE | 16 | 15 | 2 |
| **合计** | **73** | **64** | **10** |
