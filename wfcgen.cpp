#include "hyper.h"

namespace hr {

EX namespace wfc {

typedef map<vector<eWall>, short> wfc_data;

wfc_data probs;

typedef pair<const vector<eWall>, short> probdata;

wfc_data gen_decompressed(const wfc_data& data) {
  wfc_data res;
  for(auto& d: data) {
    auto cur = d.first;
    for(int i=0; i<isize(cur)-1; i++) {
      for(int k=2; k<isize(cur); k++) swap(cur[k-1], cur[k]);
      res[cur] += d.second / (isize(cur)-1);
      }
    }
  return res;
  }

wfc_data gen_compressed(const wfc_data& data) {
  wfc_data res;
  for(auto& d: data) {
    vector<eWall> cur = d.first;
    vector<eWall> best = d.first;
    for(int i=0; i<isize(best)-1; i++) {
      for(int k=2; k<isize(cur); k++) swap(cur[k-1], cur[k]);
      if(cur < best) best = cur;
      }
    res[best] += d.second;
    }
  return res;
  }

void hwrite(hstream& hs, wfc_data& data) {
  int siz = isize(data);
  hr::hwrite(hs, siz);
  for(auto p: data) {
    hr::hwrite<char>(hs, isize(p.first));
    for(auto c: p.first) hr::hwrite<char>(hs, c);
    hr::hwrite<short>(hs, p.second);
    }
  }

void hread(hstream& hs, wfc_data& data) {
  data.clear();
  int qty = hs.get<int>();
  for(int i=0; i<qty; i++) {
    vector<eWall> w;
    w.resize(hs.get<char>());
    for(auto& c: w) c = eWall(hs.get<char>());
    data[w] = hs.get<short>();
    }
  }

EX void load_probs() {
  start_game();
  manual_celllister cl;
  cl.add(cwt.at);
  for(int i=0; i<isize(cl.lst); i++) {
    cell *c = cl.lst[i];
    if(c->wall == waChasm || c->mpdist > 7) goto next;
    forCellEx(c1, c) if(c1->wall == waChasm || c1->mpdist > 7) goto next;
    
    for(int i=0; i<c->type; i++) {
      vector<eWall> s;
      if(c->type == 6 && (i&1)) continue;
      s.push_back(c->wall);
      for(int j=0; j<c->type; j++) s.push_back(c->modmove(i+j)->wall);
      bool interesting = false;
      for(eWall w: s) if(w != waNone) interesting = true;
      if(interesting) probs[s] += 42 / c->type;
      }
    
    forCellEx(c1, c) cl.add(c1);
    next: ;
    }
  }

bool agree(cell *c, probdata& p) {
  // println(hlog, p);
  if(isize(p.first) != c->type + 1) return false;
  int idx = 1;
  if(c->wall != waChasm && eWall(c->wparam) != p.first[0]) return false;
  forCellEx(c1, c) {
    eWall found = p.first[idx++];
    if(c1->wparam != found && c1->wall != waChasm) return false;
    }
  return true;
  }

vector<probdata*> gen_picks(cell *c, int& total, wfc_data& data) {
  vector<probdata*> picks;
  total = 0;
  
  for(auto& wp: data) {
    if(!agree(c, wp)) continue;
    picks.push_back(&wp);
    total += wp.second;
    }
  
  return picks;
  }

EX vector<cell*> centers;

EX void schedule(cell *c) {
  centers.push_back(c);
  }

void generate_all() {
  start_game();
  manual_celllister cl;
  cl.add(cwt.at);
  vector<cell*> centers;
  for(int i=0; i<isize(cl.lst); i++) {
    cell *c = cl.lst[i];
    bool incenter = true;
    forCellEx(c1, c) {
      if(c1->mpdist <= 7) 
        cl.add(c1); 
      else 
        incenter = false;
      }
    if(incenter) 
      centers.push_back(c);
    }
  
  for(cell *c: cl.lst) c->wall = waChasm;
  }

string eclectic_c = string("\x78\xda\x8d\x9a\x3d\x72\xe4\x36\x10\x85\x07\x4e\xc8\xd4\x01\x39\x09\x92\x4d\x1c\x80\x4c\x00\x12\xf1\x44\x14\xd2\xad\x2d\x5d\x42\x89\x42\x87\xba\x82\x83\xbd\xc4\x5e\xc2\x17\x70\xa8\xf3\x6c\xe0\xd1\x90\x1c\x02\x8d\xee\xd7\x54\x15\x6a\x58\xa5\x4f\xaf\x1b\x3f\xfd\x06\x04\xf4\xcd\x5c\x2e\xed\x05\xff\xdc\x91\xcb\x2f\x95\xea\xef\xcd\xa9\xd4\xf5\x14\x65\x4f\x51\xee\xde\x7e\xab\xd4\x70\x4a\xcb\xdf\xdb\xdf\x7f\x68\x54\x3c\x35\x12\xcb\xa9\x88\x2f\xa7\xb4\xd2\xbd\x7d\xaa\xd4\xfb\x83\x6a\x94\x39\xfc\x17\xea\x98\xad\x39\xa8\xd3\xab\x59\xf7\x5b\xfb\x0d\x75\xbe\xd6\xc1\x1b\x24\xec\x83\x68\x15\xc2\xaa\x19\x3b\x55\xc7\x3d\x57\x49\xa3\xac\xa3\x9f\x50\x67\xd8\xb4\x9c\x4a\x7d\xb5\x57\x95\x1a\x1f\x14\xca\x69\x54\x73\x1a\x4f\x55\xc0\x78\x2a\xda\x57\x8d\xfc\x07\x6b\xc4\x3f\x2b\x09\xe9\x84\x7b\xeb\x60\x3e\x61\x6b\x78\x46\x66\x75\xd5\xc7\xc7\xaa\x6f\x94\x5a\xfd\x6e\x1a\xa5\x4e\xf1\x4a\xd5\x6b\x34\x6d\x0d\x8f\xef\x8f\xc7\x6c\x36\x4a\x9d\xe3\xea\x7b\xdf\x9a\x3c\x72\x7b\x9d\xff\x65\x5a\x85\xc0\x6e\xb0\xd7\xf9\x87\x98\x4f\x9f\x35\xb9\xe7\xd7\xe7\x77\x47\xa3\xd4\xb9\x5c\x31\xf6\x94\x1b\xd8\x6d\x45\x74\x22\xa1\x3b\xc1\x5e\xe3\x1d\x24\xbe\xda\x0d\x12\x23\xac\x81\x21\xab\x49\x39\x97\xbd\xb6\x65\x9d\xb1\x70\xa5\x46\xad\xfe\x56\x21\xb0\x93\x8c\x45\xd6\x2d\xf0\x88\x63\x27\xd0\x28\x4e\xf2\x8f\x69\x15\x22\xaa\x3a\x01\xce\x97\xdf\x9c\x04\x6b\xe0\x75\x13\x36\x0a\x13\xd8\xf7\xe6\x62\x0d\x23\x6a\xef\xb9\x9c\xf1\xac\x66\x1c\x9f\x19\xb7\x0a\x81\x23\xc5\x2d\x9a\x1c\x69\xd9\xbc\xef\x26\x46\x7a\xc9\x5c\xd2\xa9\x14\x76\xb7\xb4\x45\xbc\x41\x22\xc1\x9e\xa7\x4c\xc7\xa9\x54\x82\x95\xf3\xae\xee\x09\x4d\xb5\x73\x6c\x00\x65\xc4\xcc\x4d\x46\xc9\xf5\x9e\x7b\x32\xaf\x63\x4f\xed\xae\x72\xc7\xe5\xe7\xde\x15\x4e\xd7\x00\xa7\x74\x2a\x21\xbb\xdc\x78\xca\xe5\x72\x67\xe2\x23\xf9\xac\x72\x64\x62\x6f\xaf\x90\x08\xe2\xfa\x0b\x19\x25\x13\x01\x3a\xd2\x4e\xc8\xbe\x37\x65\x51\xf8\x51\x9b\x08\xc5\xeb\xcc\x59\xbf\x3a\x91\xc0\x7b\xb5\xa8\x12\x4b\x56\x4f\x1d\x24\xde\xc5\xfe\x2c\xd5\xee\xa7\x05\x0e\x72\x81\x54\x22\x7f\xc1\x67\x9d\x54\xcf\x4a\xc4\x27\x64\x9d\x05\x3a\x52\x22\x8d\xd7\xc9\x7d\xa6\x83\xc4\x0b\x4b\x98\x6c\xae\x27\x91\xc8\x5d\xa8\xf6\x73\x4a\xf0\xfb\xc7\xdc\x81\xae\x6c\xa4\xfd\x37\x17\x71\x0d\x5b\xe6\xb3\x8e\xa4\x3b\x99\x25\x54\x07\x09\x39\x17\xab\xe6\xe2\xc8\xb8\xd4\x3a\xbb\x5f\xe2\xef\x02\x4a\xd4\x19\xfb\xcc\xcd\x2c\x24\xbc\xb8\x27\xa1\x44\x60\x57\x79\x4e\xed\x8e\x22\x53\x21\x73\x84\xba\x5f\x81\xe8\xd5\x3a\x91\x21\x6b\x9d\x98\xf5\xde\xb0\xbd\x5f\x48\x6d\xf1\x44\xb9\x07\x68\x80\x93\xf0\x1a\x29\x73\x80\x17\x76\x8c\x93\x5a\xdb\x86\xf9\x2c\xc7\x85\x23\x0c\xab\x43\x89\x0f\x96\x30\xa2\xe3\x73\x15\x27\x13\x7b\xbb\x41\x82\x8f\x42\xa9\xd2\x67\x2c\x53\x93\xf5\x7a\xd9\x89\x7c\xb7\x5c\xd7\xa5\x27\xd5\x52\xef\x98\xe9\xda\x9d\xab\x99\xa4\x55\xc2\xaf\xf0\xfc\x5d\x85\xdf\x35\x05\x92\xcb\x0c\x09\xfe\x8d\x24\x90\x4c\x31\x11\xd9\xfe\x04\x26\x97\xba\x3f\x91\xe9\x79\xa9\xb3\x90\x1a\xa8\x57\xb8\x11\xda\x31\x93\x3d\x68\x87\x8e\x27\x59\xd7\xf9\x24\xd0\xd6\x68\x46\x39\x17\x5d\xa3\x19\x40\x74\xa2\x4e\x5e\x79\x9f\x2c\x65\xc8\xf3\x27\x1b\x2d\xd7\xf9\x09\x75\x8c\x98\x35\x7d\x7e\x63\x89\xfa\x5d\xa3\x17\xde\x24\x2e\xc5\x6c\xf4\x80\x78\x65\x75\x7a\x32\xd3\x8e\xa5\xae\xd5\x2d\x43\x03\xa8\xfe\xb9\xea\x7b\x26\xd2\xfe\x7c\x13\x89\x1e\x6a\xf4\x62\x14\xee\x73\x1d\xbd\xab\x7a\x6b\xd2\x00\xea\xd0\xb1\xea\x8d\x49\x03\xa8\xe3\x5c\xc1\x82\xb7\xbe\x08\x75\x0e\xef\xe2\x74\x2c\x79\xfe\xc5\xea\xe4\xfe\x7e\xab\x08\xab\x66\x6c\xc5\x1b\x00\xab\xee\xec\xac\xba\x4f\xb2\xc2\xf7\xdd\xe1\x94\xee\xd4\xd9\xa1\x03\x27\x7b\x6f\x22\x51\xbe\x0d\x0f\xe0\x9c\x6d\xcd\x65\x50\x4e\xe2\x1c\x4b\xb9\x2a\x62\x03\x28\x39\xda\xfe\x76\x5f\x9e\x38\x0c\xea\xbb\xf7\x00\x72\xa9\x23\x39\xb2\xf3\xe5\x33\x1e\x60\x9f\x1c\xe9\xcf\x11\x69\x54\x6e\x45\x56\x9d\x11\x9c\xcf\x76\x2c\x31\x16\x91\x10\xc1\x6b\x0c\xd5\x7c\x73\xc4\xa0\x12\x65\x14\xda\xe3\x91\x5d\x33\x8d\xf0\x9b\x8b\x4a\xf0\x3d\x1e\xd8\x1e\xb7\x19\x3f\x0a\x6d\x5d\xbf\x5e\xb9\x71\x5a\xf7\xb2\x1e\x10\x37\x91\xd8\x5d\xe0\x4f\x23\x11\xa1\x78\xf3\xf3\x8c\x9b\xf9\x4c\xcb\xb1\x91\x4a\xa2\x55\x08\x4c\xe5\xe4\xab\x48\x4d\xc5\xc9\x36\x47\x45\xb2\x7f\x92\x29\x9f\x9d\x1a\xd5\xfd\xa3\xcf\xb5\x8e\x67\x7a\xf0\x21\xea\xec\xe3\xfd\x26\xea\x04\xd8\x37\x4a\x45\x98\xf5\x24\xce\xae\x67\xc6\x89\xd7\xb1\xd5\x5b\x93\x17\xde\x10\x64\x22\x64\xcf\x7c\xcf\x03\xe9\x97\x13\x75\x3c\x8c\x14\xc5\x6c\xb9\x4f\x9e\x08\xc5\xba\xe1\x88\xb9\xba\x7b\x91\x88\x49\x24\x62\x75\x46\xe1\x85\x3a\xf0\xec\x98\x04\xf1\xac\x23\x30\x35\x5e\x57\x5f\x10\x2a\x6f\xff\x8b\x75\x7f\x23\x51\x01\xea\x94\x14\xd5\xa1\xcf\x9d\x48\x1c\xa7\xc9\xad\x42\xf8\x2a\x52\x20\xa7\xb8\x51\x24\x62\x75\x2a\x3d\x09\xf5\x7d\xcc\xe6\x24\xf4\x78\x7a\xce\xc2\x0c\x7c\x3d\x3e\xdd\x6d\x56\x6e\xd4\xd6\xb3\xf8\x19\x78\x7b\x57\x11\xfc\xde\x2f\x82\x6f\x90\x35\xe3\x78\x2a\x97\x08\x6e\xe5\x9c\x48\x94\x73\x14\x85\x5d\xf3\x91\xcb\x72\xc1\x3f\x4e\xa5\x8e\xff\x48\x58\x00\x71\x13\x75\xca\x33\xf0\x86\x9c\x8b\xd1\x9f\xf5\x3b\x38\x01\xa2\x83\xc4\x91\x4b\x52\x6e\x11\x9c\x48\xa5\xea\x6c\x35\xa9\xb7\x11\x09\x68\x60\xe2\x18\xdf\x04\xce\x1f\xcb\x3c\x16\x46\xa3\x8c\xc2\x7d\xae\xe3\xf2\x43\x5d\x11\x4d\x76\x0f\x20\xcf\x91\x44\xac\x77\x26\xff\x03\xaa\x5b\xdc\x2f", 1265);

void extend(wfc_data& res, vector<eWall> v, int val, int sides) {
  if(isize(v) == sides+1)
    res[v] += val;
  else if(isize(v) < sides + 1) {
    for(int i=1; i<isize(v); i++) {
      auto w = v;
      w.insert(w.begin()+i, v[i]);
      extend(res, w, val, sides);
      }
    v.insert(v.begin()+1, v.back());
    extend(res, v, val, sides);
    }
  else {
    for(int i=1; i<isize(v); i++) {      
      auto w = v;
      auto l = v[i==1 ? isize(v)-1 : i-1];
      auto r = v[i == isize(v)-1 ? 1 : i+1];
      if(l == waCharged && r == waGrounded) continue;
      if(l == waGrounded && r == waCharged) continue;
      w.erase(w.begin()+i);
      extend(res, w, val, sides);
      }
    }
  }

wfc_data renumerate(const wfc_data& d, int sides) {
  wfc_data res;
  for(auto& p: d) {
    extend(res, p.first, p.second, sides);
    }
  return res;
  }

wfc_data& eclectic_data() {
  static wfc_data d = gen_decompressed(deserialize<wfc_data>(decompress_string(eclectic_c)));
  if(geometry == gNormal && !PURE) return d;
  if(geometry == gNormal && PURE) {
    static wfc_data dpure = renumerate(d, 7);
    return dpure;
    }
  if(geometry == gEuclid) {
    static wfc_data deuc = renumerate(d, 6);
    return deuc;
    }
  return d;
  }

EX bool use_eclectic = true;

EX void invoke() {
  wfc_data& d = use_eclectic ? eclectic_data() : probs;

  while(isize(centers)) {
    int pos = -1;
    ld best_entropy = 1e9;
    for(int p=0; p<isize(centers); p++) {
      cell *c = centers[p];
      int total;
      
      auto picks = gen_picks(c, total, d);
      
      ld entropy = 0;
      for(auto p: picks) entropy += p->second * log(total * 1. / p->second) / total;
      
      if(entropy < best_entropy) best_entropy = entropy, pos = p;
      }
    
    cell *c = centers[pos];
    centers[pos] = centers.back();
    centers.pop_back();

    // println(hlog, "chosen ", c, " at entropy ", best_entropy, " in distance ", c->mpdist);

    int total;
    auto picks = gen_picks(c, total, d);

    if(total) total = hrand(total);
    for(auto pp: picks) {
      auto& p = *pp;
      total -= p.second;
      if(total < 0) {
        int idx = 1;
        c->wall = p.first[0];
        c->wparam = p.first[0];
        forCellEx(c1, c) {
          if(c1->wall != waBarrier) 
            c1->wparam = c1->wall = p.first[idx];
          idx++;
          }
        break;
        }
      }

    }

  }

void use_probs() {
  stop_game();
  use_eclectic = false;
  firstland = specialland = laEclectic;
  start_game();
  }

EX void wfc_menu() {
  cmode = sm::SIDE | sm::MAYDARK;
  gamescreen();
  dialog::init(XLAT("Wave Function Collapse"));
  
  dialog::addSelItem(XLAT("import the current map"), XLAT("rules: %1", its(isize(probs))), 'i');
  dialog::add_action(load_probs);

  dialog::addBoolItem(XLAT("generate a map using WFC"), !use_eclectic, 'g');
  if(probs.empty()) dialog::lastItem().value = "(no rules)";
  
  dialog::add_action([] {
    if(probs.empty()) return;
    if(use_eclectic) {
      use_probs();
      }
    else {
      stop_game();
      use_eclectic = true;
      start_game();
      }
    });
  
  dialog::addBack();
  dialog::display();
  }

#if CAP_COMMANDLINE
auto wfc_hook = 
  // addHook(hooks_handleKey, 100, wfc_handleKey) +
  addHook(hooks_args, 100, [] {
    using namespace arg;

    if(0) ;
    else if(argis("-wfc-load")) {
      load_probs();
      }
    else if(argis("-wfc-c")) {
      println(hlog, as_cstring(compress_string(serialize(gen_compressed(probs)))));
      }
    else if(argis("-wfc-use")) {
      use_probs();
      }
      
    else return 1;
    return 0;
    });
#endif

auto cgm = addHook(hooks_clearmemory, 40, [] () { centers.clear(); }) +  addHook(hooks_removecells, 0, [] () { eliminate_if(centers, is_cell_removed); });

EX }

}
