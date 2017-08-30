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


local refimg = rb.texture2(16, 24)
for u = 0, 15 do
    for v = 0, 23 do
        local r = 0
        local g = 0
        local b = 0
        if u == 7 or u == 8 then r = 1 end
        if v == 12 or v == 13 then g = 1 end
        if u == 0 or v == 0 or u == 15 or v == 23 then b = 1 end
        refimg:sett(u, v, rb.color(r, g, b))
    end
end

local comp = rb.gen_compositor()
comp:add_layer(rb.gen_static_image(refimg),
    rb.texture2(16, 24))
rb.set_generator(comp)


--rb.set_generator(rb.gen_fill(1))

function generate_parameters(analysis, parameters)
--    print('hi', analysis.frameNum, analysis.time, analysis.controls.controllers[1], analysis.controls.triggers[1])
    parameters[1] = 0
    parameters[2] = 0.5
    parameters[3] = 0.5
end
