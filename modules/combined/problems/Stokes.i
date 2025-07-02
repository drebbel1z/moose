param1 = 1.0 # (varying)
param2 = 0.0
param3 = 0.0
param4 = 2.5 # (varying)

## Parameters (true values)
rho1 = 1.0
mu1 = 0.015

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 1.0
    nx = 40
    ny = 40
    # elem_type = QUAD9
  []
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  [../]
  second_order = true
[]

[AuxVariables]
  [vel_x]
    order = SECOND
  []
  [vel_y]
    order = SECOND
  []
[]

[AuxKernels]
  [vel_x]
    type = VectorVariableComponentAux
    variable = vel_x
    vector_variable = velocity
    component = 'x'
  []
  [vel_y]
    type = VectorVariableComponentAux
    variable = vel_y
    vector_variable = velocity
    component = 'y'
  []
[]

[Variables]
  [./velocity]
    order = SECOND
    family = LAGRANGE_VEC
  [../]
  [./p]
  [../]
[]

[Kernels]
  [./mass]
    type = INSADMass
    variable = p
  [../]
  [./momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  [../]
  [./momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
    integrate_p_by_parts = true
  [../]
[]

[BCs]
  [./lid]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'top'
    values = '${param1} 0.0 0.0'
  [../]
  [./lid1]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'bottom'
    values = '${param2} 0.0 0.0'
  [../]
  [./lid2]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'left'
    values = '0.0 ${param3} 0.0'
  [../]
  [./lid3]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'right'
    values = '0.0 ${param4} 0.0'
  [../]

  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0.0
  [../]
[]

[Materials]
  [./const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho1} ${mu1}'
  [../]
  [ins_mat]
    type = INSADMaterial
    velocity = velocity
    pressure = p
  []
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  nl_rel_tol = 1e-12
  nl_max_its = 100
  l_max_its = 20
  automatic_scaling = true
[]

[Functions]
  [log_res_vel1]
    type = ParsedFunction
    expression = '(sqrt(a^2 + b^2))'
    symbol_names = 'a b'
    symbol_values = 'vel_x1 vel_y1'
  []
  [log_res_vel2]
    type = ParsedFunction
    expression = '(sqrt(a^2 + b^2))'
    symbol_names = 'a b'
    symbol_values = 'vel_x2 vel_y2'
  []
  [log_res_vel3]
    type = ParsedFunction
    expression = '(sqrt(a^2 + b^2))'
    symbol_names = 'a b'
    symbol_values = 'vel_x3 vel_y3'
  []
  [log_res_vel4]
    type = ParsedFunction
    expression = '(sqrt(a^2 + b^2))'
    symbol_names = 'a b'
    symbol_values = 'vel_x4 vel_y4'
  []
  [log_res_vel5]
    type = ParsedFunction
    expression = '(sqrt(a^2 + b^2))'
    symbol_names = 'a b'
    symbol_values = 'vel_x5 vel_y5'
  []
  [log_res_vel6]
    type = ParsedFunction
    expression = '(sqrt(a^2 + b^2))'
    symbol_names = 'a b'
    symbol_values = 'vel_x6 vel_y6'
  []
  [log_res_vel7]
    type = ParsedFunction
    expression = '(sqrt(a^2 + b^2))'
    symbol_names = 'a b'
    symbol_values = 'vel_x7 vel_y7'
  []
  [log_res_vel8]
    type = ParsedFunction
    expression = '(sqrt(a^2 + b^2))'
    symbol_names = 'a b'
    symbol_values = 'vel_x8 vel_y8'
  []
  [log_res_vel9]
    type = ParsedFunction
    expression = '(sqrt(a^2 + b^2))'
    symbol_names = 'a b'
    symbol_values = 'vel_x9 vel_y9'
  []
  [log_inv_error]
    type = ParsedFunction
    expression = 'log(1/((a-0.253)^2 + (b-0.576)^2 + (c-0.916)^2 + (d-0.187)^2 + (f-0.280)^2 + (g-0.134)^2 + (h-0.444)^2 + (i-0.645)^2))'
    symbol_names = 'a b c d f g h i'
    symbol_values = 'log_resultant_velocity1 log_resultant_velocity2 log_resultant_velocity3 log_resultant_velocity4 log_resultant_velocity6 log_resultant_velocity7 log_resultant_velocity8 log_resultant_velocity9'
  []
[]

[Postprocessors]
  [vel_x1]
    type = PointValue
    point = '0.25 0.75 0.0'
    variable = vel_x
  []
  [vel_y1]
    type = PointValue
    point = '0.25 0.75 0.0'
    variable = vel_y
  []
  [vel_x2]
    type = PointValue
    point = '0.5 0.75 0.0'
    variable = vel_x
  []
  [vel_y2]
    type = PointValue
    point = '0.5 0.75 0.0'
    variable = vel_y
  []
  [vel_x3]
    type = PointValue
    point = '0.75 0.75 0.0'
    variable = vel_x
  []
  [vel_y3]
    type = PointValue
    point = '0.75 0.75 0.0'
    variable = vel_y
  []
  [vel_x4]
    type = PointValue
    point = '0.25 0.5 0.0'
    variable = vel_x
  []
  [vel_y4]
    type = PointValue
    point = '0.25 0.5 0.0'
    variable = vel_y
  []
  [vel_x5]
    type = PointValue
    point = '0.5 0.5 0.0'
    variable = vel_x
  []
  [vel_y5]
    type = PointValue
    point = '0.5 0.5 0.0'
    variable = vel_y
  []
  [vel_x6]
    type = PointValue
    point = '0.75 0.5 0.0'
    variable = vel_x
  []
  [vel_y6]
    type = PointValue
    point = '0.75 0.5 0.0'
    variable = vel_y
  []
  [vel_x7]
    type = PointValue
    point = '0.25 0.25 0.0'
    variable = vel_x
  []
  [vel_y7]
    type = PointValue
    point = '0.25 0.25 0.0'
    variable = vel_y
  []
  [vel_x8]
    type = PointValue
    point = '0.5 0.25 0.0'
    variable = vel_x
  []
  [vel_y8]
    type = PointValue
    point = '0.5 0.25 0.0'
    variable = vel_y
  []
  [vel_x9]
    type = PointValue
    point = '0.75 0.25 0.0'
    variable = vel_x
  []
  [vel_y9]
    type = PointValue
    point = '0.75 0.25 0.0'
    variable = vel_y
  []
  [log_resultant_velocity1]
    type = FunctionValuePostprocessor
    function = 'log_res_vel1'
  []
  [log_resultant_velocity2]
    type = FunctionValuePostprocessor
    function = 'log_res_vel2'
  []
  [log_resultant_velocity3]
    type = FunctionValuePostprocessor
    function = 'log_res_vel3'
  []
  [log_resultant_velocity4]
    type = FunctionValuePostprocessor
    function = 'log_res_vel4'
  []
  [log_resultant_velocity9]
    type = FunctionValuePostprocessor
    function = 'log_res_vel9'
  []
  [log_resultant_velocity5]
    type = FunctionValuePostprocessor
    function = 'log_res_vel5'
  []
  [log_resultant_velocity6]
    type = FunctionValuePostprocessor
    function = 'log_res_vel6'
  []
  [log_resultant_velocity7]
    type = FunctionValuePostprocessor
    function = 'log_res_vel7'
  []
  [log_resultant_velocity8]
    type = FunctionValuePostprocessor
    function = 'log_res_vel8'
  []
  [log_inverse_error]
    type = FunctionValuePostprocessor
    function = 'log_inv_error'
  []  
[]

[VectorPostprocessors]
  [objective_functions]
    type = VectorOfPostprocessors
    postprocessors = 'log_inverse_error'
    outputs = none
  []
[]

[Outputs]
  exodus = false
  perf_graph = false
  csv = false
  console = false
[]