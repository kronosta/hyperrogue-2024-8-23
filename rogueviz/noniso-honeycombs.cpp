#include "rogueviz.h"

namespace rogueviz {

namespace honey {

bool alone = true;

bool in_special = false;

#if CAP_RVSLIDES

using ccolor::data;

#define CCO [] (cell *c, data& cco) -> color_t

int jhole = 0;
int jblock = 0;

data jmap("rainbow by distance", ccolor::always_available,
    CCO {
      if(c == currentmap->gamestart()) return ccolor::plain(c);
      int d = c->master->distance;
      if(geometry == gNil) d = c->master->zebraval;
      if(euc::in()) d = euc::get_ispacemap()[c->master][0];
      if(d % 2 == 0 || d < -5 || d > 5) return hrand(100) < jblock ? 0xFFFFFFFF : ccolor::plain(c);
      return hrand(100) < jhole ? ccolor::plain(c) : cco.ctab[(d+5)/2];
      },
    {0x100FFFF, 0x100FF00, 0x1FFFF00, 0x1FF8000, 0x1FF0000, 0x1FF00FF}
    );

data jmap2("rainbow by distance II", ccolor::always_available,
    CCO {
      if(c == currentmap->gamestart()) return ccolor::plain(c);
      int d = c->master->distance;
      if(geometry == gNil) d = c->master->zebraval;
      if(euc::in()) d = euc::get_ispacemap()[c->master][0];
      if((d&3) != 2) return hrand(100) < jblock ? 0xFFFFFFFF : ccolor::plain(c);
      return hrand(100) < jhole ? ccolor::plain(c) : cco.ctab[(d+10)/4];
      },
    {0x100FFFF, 0x100FF00, 0x1FFFF00, 0x1FF8000, 0x1FF0000, 0x1FF00FF}
    );

data random_pseudohept("random_pseudohept", ccolor::always_available,
  CCO {
    color_t r = hrand(0xFFFFFF + 1);
    if(hrand(100) < ccolor::rwalls && pseudohept(c) && c != cwt.at) r |= 0x1000000;
    return r;
    }, {});

auto geoslide(eGeometry g, ccolor::data *canvas, int _jhole, int _jblock) {
  using namespace tour;
  return [=] (presmode mode) {
    setWhiteCanvas(mode, [&] {
      set_geometry(g);
      if(g == gSphere) {
        set_geometry(gProduct);
        }
      if(g == gNormal) {
        set_geometry(gTwistedProduct);
        }
      tour::slide_backup<ld>(sightranges[gProduct], 12);
      tour::slide_backup<ld>(sightranges[gNil], 7);
      tour::slide_backup<ld>(sightranges[gSol], 7);
      tour::slide_backup<ld>(sightranges[gSpace435], 7);
      vid.texture_step = 4;
      tour::slide_backup(jhole, _jhole);
      tour::slide_backup(jblock, _jblock);
      tour::slide_backup(ccolor::rwalls, _jhole);
      tour::slide_backup(ccolor::which, canvas);
      tour::slide_backup(vid.linewidth, vid.linewidth / 10);
      if(jblock < 0) {
        pmodel = mdDisk;
        sightranges[gSol] = 4;
        }
      });
    rogueviz::pres::non_game_slide_scroll(mode);
    if(mode == pmStop && jblock < 0)
      pmodel = mdGeodesic;
    slidecommand = "switch raycaster";

    if(in_special && among(mode, pmGeometrySpecial, pmStop)) {
      in_special = false;
      gamestack::pop();
      ccolor::which = canvas;
      vid.grid = false;
      fat_edges = false;
      sightranges[gSpace435] = 7;
      }

    else if(mode == pmGeometrySpecial && !in_special) {
      in_special = true;
      gamestack::push();
      ccolor::set_plain(0);
      vid.grid = true;
      stdgridcolor = 0xFFFF00FF;
      fat_edges = true;
      start_game();
      sightranges[gSpace435] = 3;
      }

    if(mode == pmKey && jblock < 0) {
      sightranges[gSol] = 11 - sightranges[gSol];
      addMessage("Changed the sight range to ", sightranges[gSol]);
      }
    else if(mode == pmKey) {
      if(sl2) {
        addMessage("Raycaster not implemented here.");
        }
      #if CAP_RAY
      else if(ray::want_use != 2) {
        ray::want_use = 2;
        ray::max_cells = 4096;
        addMessage("Using a raycaster.");
        }
      #endif
      else {
        #if CAP_RAY
        ray::want_use = 0;
        #endif
        addMessage("Using primitives.");
        }
      }
    };
  }
#endif

string cap = "honeycombs/";

#if CAP_RVSLIDES
void honey(string s, vector<tour::slide>& v) {
  if(s != "noniso") return;
  using namespace tour;

  v.emplace_back(
    slide{cap+"Tessellations in 3D geometries", 999, LEGAL::NONE | QUICKSKIP, 
      "This series of slides presents the honeycombs we use. "
      "You can compare the output of primitive-based and raycaster-based rendering by pressing '5'.",
      [] (presmode mode) {}
      });

  v.emplace_back(
    slide{cap+"Hyperbolic space", 999, LEGAL::SPECIAL,
      "Traditional visualizations of non-Euclidean honeycombs "
      "show the edges of all cells. In our visualizations, we fill some of the cells. "
      "The disadvantage of the traditional visualization is visible here, on the example of {4,3,5} hyperbolic honeycomb: "
      "our Euclidean brains tend to interpret this visualization incorrectly. (Press '2' to get the traditional visualization.)",
      geoslide(gSpace435, &ccolor::random, 50, 0)
      });
  v.emplace_back(
    slide{cap+"S2xE", 999, LEGAL::NONE,
      "This is the S2xE geometry.",
      geoslide(gSphere, &ccolor::random, 10, 0)
      });
  v.emplace_back(
    slide{cap+"Solv: random", 999, LEGAL::NONE,
      "Random blocks in Solv geometry.",
      geoslide(gSol, &ccolor::random, 20, 0)
      });
  v.emplace_back(
    slide{cap+"Solv: Poincaré ball", 999, LEGAL::NONE,
      "Surfaces of constant 'z' in Solv geometry, displayed in Poincaré ball-like model. "
      "Press '5' to change the sight range (this slide is not optimized, so it will be slow).",
      geoslide(gSol, &jmap, 0, -1)
      });
  v.emplace_back(
    slide{cap+"Solv: horotori", 999, LEGAL::NONE,
      "Solv geometry. Colored torus-like surfaces are surfaces of constant 'z'. "
      "Press '5' to enable the raycaster",
      geoslide(gSol, &jmap, 50, 0)
      });
  v.emplace_back(
    slide{cap+"Solv: difficult region", 999, LEGAL::NONE,
      "This slide focuses on the area in Solv geometry which is difficult to render using primitives. "
      "Press '5' to enable the raycaster.",      
      geoslide(gSol, &jmap2, 0, 10)
      });
  v.emplace_back(
    slide{cap+"Nil geometry", 999, LEGAL::NONE,
      "Nil geometry. Colored surfaces are surfaces of constant 'x'. "
      "Press '5' to enable the raycaster",
      geoslide(gNil, &jmap, 10, 10)
      });
  v.emplace_back(
    slide{cap+"SL(2,R) geometry", 999, LEGAL::NONE,
      "SL(2,R) geometry.",
      geoslide(gNormal, &random_pseudohept, 90, 0)
      });

  }
#endif

#if CAP_RVSLIDES
vector<tour::slide> noniso_slides;
tour::slide *gen_noniso_demo() {
  noniso_slides.clear();
  using namespace tour;
  noniso_slides.emplace_back(
    slide{"Non-isotropic geometry demo", 999, LEGAL::NONE | QUICKSKIP | QUICKGEO, 
      "This is a presentation of non-isotropic geometries.",
      [] (presmode mode) {
        slide_url(mode, 'p', "paper about non-isotropic geometries", "https://arxiv.org/abs/2002.09533");
        setCanvas(mode, &ccolor::random, [] {
          set_geometry(gCubeTiling);
          });
        }
      });

  callhooks(pres::hooks_build_rvtour, "noniso", noniso_slides);
  pres::add_end(noniso_slides);
  return &noniso_slides[0];
  }
#endif

auto hooks  = addHook_rvslides(163, honey)
  + addHook_slideshows(120, [] (tour::ss::slideshow_callback cb) {
  
    if(noniso_slides.empty()) 
      gen_noniso_demo();

    cb(XLAT("non-isotropic geometries"), &noniso_slides[0], 'n');
    });

} }
