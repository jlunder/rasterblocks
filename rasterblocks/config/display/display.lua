include('palettes.lua')

--[[
gen_compositor(generator...)
gen_static_image(image)
gen_image_filter(generator, image)
gen_rescale(generator, source_width, source_height)
gen_timed_rotation({generator...}, auto_switch_time, manual_switch_trigger_num)
gen_controller_select({generator...}, controller_num)
gen_controller_fade(generator, controller_num)
gen_trigger_flash(palette, trigger_num)
gen_plasma(palette)
gen_beat_flash(palette)
gen_pulse_plasma(palette)
gen_pulse_grid(horizontal_color, vertical_color)
gen_dashed_circles(palette)
gen_smoke_signals(palette)
gen_fireworks(palette, trigger_num)
gen_vertical_bars(bass_palette, treble_palette, num_bars, spawn_time, fade_time)
gen_criscross(palette, num_bars, spawn_time, fade_time)
gen_volume_bars(bass_palette, treble_palette)
gen_beat_stars(color)
gen_icon_checkerboard(color)
gen_pulse_checkerboard(color)
gen_particle_lissajous(color)
gen_signal_lissajous(color)
gen_oscilloscope(color)
]]

function make_gen_set(back_pal, fore_pal, back_col, fore_col)
  return rb.gen_controller_select({
    rb.gen_compositor(
      rb.gen_pulse_plasma(back_pal),
      rb.gen_dashed_circles(fore_pal)),
    rb.gen_dashed_circles(back_pal),
    rb.gen_compositor(
      rb.gen_vertical_bars(back_pal, fore_pal,
        8, rb.time(.2), rb.time(1)),
      rb.gen_dashed_circles(fore_pal)),
    
    rb.gen_compositor(
      rb.gen_pulse_plasma(back_pal),
      rb.gen_pulse_checkerboard(fore_col)),
    rb.gen_compositor(
      rb.gen_dashed_circles(back_pal),
      rb.gen_pulse_checkerboard(fore_col)),
    rb.gen_compositor(
      rb.gen_vertical_bars(back_pal, fore_pal,
        8, rb.time(.2), rb.time(1)),
      rb.gen_pulse_checkerboard(fore_col)),
    
    rb.gen_compositor(
      rb.gen_pulse_plasma(back_pal),
      rb.gen_pulse_checkerboard(fore_col)),
    rb.gen_compositor(
      rb.gen_dashed_circles(back_pal),
      rb.gen_pulse_checkerboard(fore_col)),
    rb.gen_compositor(
      rb.gen_vertical_bars(back_pal, fore_pal,
        8, rb.time(.2), rb.time(1)),
      rb.gen_pulse_checkerboard(fore_col)),
    
    rb.gen_compositor(
      rb.gen_pulse_plasma(back_pal),
      rb.gen_pulse_checkerboard(fore_col)),
    rb.gen_compositor(
      rb.gen_dashed_circles(back_pal),
      rb.gen_pulse_checkerboard(fore_col)),
    rb.gen_compositor(
      rb.gen_vertical_bars(back_pal, fore_pal,
        8, rb.time(.2), rb.time(1)),
      rb.gen_pulse_checkerboard(fore_col)),
  }, 2)
end


rb.set_generator(
  rb.gen_controller_select(
    {
      rb.gen_compositor(
        rb.gen_controller_fade(
          rb.gen_controller_select(
            {
              rb.gen_pulse_plasma(pal_black_purple_red_hs),
              rb.gen_dashed_circles(pal_black_purple_red_hs),
              rb.gen_vertical_bars(pal_black_purple_red_hs, pal_black_red_gold_hs_add,
                8, rb.time(.2), rb.time(1)),
              rb.gen_vertical_bars(pal_black_purple_red_hs, pal_black_red_gold_hs_add,
                32, rb.time(0.01), rb.time(0.1)),
            },
            0
          ),
          1
        ),
        rb.gen_controller_select(
          {
            rb.gen_compositor(
              rb.gen_controller_fade(rb.gen_dashed_circles(pal_black_red_hs_add), 4),
              rb.gen_controller_fade(rb.gen_dashed_circles(pal_black_green_hs_add), 5),
              rb.gen_controller_fade(rb.gen_dashed_circles(pal_black_blue_hs_add), 6)
            ),
            rb.gen_compositor(
              rb.gen_controller_fade(rb.gen_pulse_checkerboard(rb.color(1, 0, 0, 0.1)), 4),
              rb.gen_controller_fade(rb.gen_pulse_checkerboard(rb.color(0, 1, 0, 0.1)), 5),
              rb.gen_controller_fade(rb.gen_pulse_checkerboard(rb.color(0, 0, 1, 0.1)), 6)
            ),
            rb.gen_compositor(
              rb.gen_controller_fade(rb.gen_oscilloscope(rb.color(1, 0, 0, 0.1)), 4),
              rb.gen_controller_fade(rb.gen_oscilloscope(rb.color(0, 1, 0, 0.1)), 5),
              rb.gen_controller_fade(rb.gen_oscilloscope(rb.color(0, 0, 1, 0.1)), 6)
            ),
            rb.gen_compositor(
              rb.gen_controller_fade(rb.gen_signal_lissajous(rb.color(1, 0, 0, 0.1)), 4),
              rb.gen_controller_fade(rb.gen_signal_lissajous(rb.color(0, 1, 0, 0.1)), 5),
              rb.gen_controller_fade(rb.gen_signal_lissajous(rb.color(0, 0, 1, 0.1)), 6)
            ),
          },
          2
        )
      ),
      rb.gen_controller_fade(
        rb.gen_controller_select(
          {
            make_gen_set(pal_black_purple_red_hs, pal_black_blue_hs_add,
              rb.color(.3, .3, 0), rb.color(0, 0, .5)),
            make_gen_set(pal_green_lavender_hs, pal_black_green_hs_add,
              rb.color(0, .3, .3), rb.color(0, .5, 0)),
          },
          0--rb.time(10), 1
        ), 1
      ),
    }, 3
  )
)
