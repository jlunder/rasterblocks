include('palettes.lua')

--[[
color(r, g, b)

gen_compositor()
compositor_add_layer(comp_gen, generator, bb_tex, blend, alpha_param,
    pos_param, scale_param)
gen_static_image(image)
gen_fill(color_param)
gen_selector({generator...}, sel_param)
gen_plasma(palette, scale_param)
gen_dashed_circles(palette, scale_param, dash_scale_param, rot_param)
gen_vertical_bars(palette, num_bars, spawn_time, fade_time, intens_param)
gen_signal_lissajous(palette)
gen_oscilloscope(palette)
]]

rb.set_generator(rb.gen_fill(1))
--rb.set_generator(rb.gen_fill(1))

function generate_parameters(analysis, parameters)
--    print('hi', analysis.frameNum, analysis.time, analysis.controls.controllers[1], analysis.controls.triggers[1])
    parameters[1] = 0
    parameters[2] = 0.5
    parameters[3] = 0.5
end
