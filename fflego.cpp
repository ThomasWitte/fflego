#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>

#include <vector>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <string>
#include <unordered_set>

using namespace std;

constexpr int PIXMAP_W = 64;
constexpr int PIXMAP_H = 64;

enum LegoPiece {_1x1, _1x2, _2x1, _1x3, _3x1, _1x4,
                _4x1, _1x6, _6x1, _1x8, _8x1, _2x2, _4x4};

struct Piece {
	LegoPiece type;
	int x;
	int y;
};

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    
    bool operator==(const Color& o) const {
        return (r == o.r) && (g == o.g) && (b == o.b);
    }
};

namespace std {
template<>
struct hash<Color> {
    size_t operator()(const Color& c) const {
        return (c.r << 16) | (c.g << 8) | c.b;
    }
};
}

constexpr int values[] = {54, 43, 43, 84, 84, 83, 83, 84, 84, 83, 83, 41, 160};
constexpr int prices[] = {7, 7, 14, 14, 14, 14, 7};
constexpr int tsize[] = {1, 2, 2, 3, 3, 4, 4, 6, 6, 8, 8, 4, 16};
constexpr int min_val_per_tile = 35;
constexpr int min_val_per_tile_opt = 10;
//constexpr int min_val_per_tile = 1;
constexpr const char* names[] = {"1 1", "1 2", "2 1", "1 3", "3 1", "1 4",
								 "4 1", "1 6", "6 1", "1 8", "8 1", "2 2", "4 4"};

struct Element {
	vector<bool> pixmap;
	int open_tiles;
	
	Piece piece;

	int value;
	bool open;

	int parent_idx;
};

int lowest_value_open(vector<Element>& elements) {
	int lowest_val = 100000000;
	int lowest_elt = 0;

    auto sz = elements.size();
	for (int i = 0; i < sz; ++i) {
	    const auto& elt = elements[i];
	    if (elt.open) {
	        int min_rem_val = elt.open_tiles * min_val_per_tile;
		    if (elt.value + min_rem_val < lowest_val && elt.open) {
			    lowest_val = elt.value + min_rem_val;
			    lowest_elt = i;
		    }
		}
	}

	return lowest_elt;
}

bool is_solved(const Element& elt) {
	return elt.open_tiles == 0;
}

tuple<int, int> get_next_free_position(const vector<bool>& pixmap) {
	int pos = find(pixmap.begin(), pixmap.end(), true) - pixmap.begin();
	return make_tuple(pos/PIXMAP_W, pos%PIXMAP_W);
}

vector<Piece> fitting_pieces(const vector<bool>& pixmap, int x, int y) {
	vector<Piece> result;
	int pos = y * PIXMAP_W + x;
	if (pos < PIXMAP_W * PIXMAP_H && pixmap[pos]) {
		result.push_back(Piece {_1x1, x, y});
		if ((pos+1)/PIXMAP_W == y && pixmap[pos+1]) {
			result.push_back(Piece {_1x2, x, y});
			if ((pos+2)/PIXMAP_W == y && pixmap[pos+2]) {
				result.push_back(Piece {_1x3, x, y});
				if ((pos+3)/PIXMAP_W == y && pixmap[pos+3]) {
					result.push_back(Piece {_1x4, x, y});
					if ((pos+5)/PIXMAP_W == y && pixmap[pos+4] && pixmap[pos+5]) {
						result.push_back(Piece {_1x6, x, y});
						if ((pos+7)/PIXMAP_W == y && pixmap[pos+6] && pixmap[pos+7]) {
							result.push_back(Piece {_1x8, x, y});
						}
					}
				}
			}
		}
		if (pos < PIXMAP_W * (PIXMAP_H-1) && pixmap[pos + PIXMAP_W]) {
			result.push_back(Piece {_2x1, x, y});
			if (pos < PIXMAP_W * (PIXMAP_H-2) && pixmap[pos + 2*PIXMAP_W]) {
				result.push_back(Piece {_3x1, x, y});
				if (pos < PIXMAP_W * (PIXMAP_H-3) && pixmap[pos + 3*PIXMAP_W]) {
					result.push_back(Piece {_4x1, x, y});
					if (pos < PIXMAP_W * (PIXMAP_H-5) && pixmap[pos + 4*PIXMAP_W] && pixmap[pos + 5*PIXMAP_W]) {
						result.push_back(Piece {_6x1, x, y});
						if (pos < PIXMAP_W * (PIXMAP_H-7) && pixmap[pos + 6*PIXMAP_W] && pixmap[pos + 7*PIXMAP_W]) {
							result.push_back(Piece {_8x1, x, y});
						}
					}
				}
			}
		}
		if ((pos+1) < PIXMAP_W * (PIXMAP_H-1) && (pos+1)/PIXMAP_W == y && pixmap[pos + PIXMAP_W] && pixmap[pos+1] && pixmap[pos+1+PIXMAP_W]) {
			result.push_back(Piece {_2x2, x, y});
			if ((pos+3) < PIXMAP_W * (PIXMAP_H-3) && (pos+3)/PIXMAP_W == y &&
			     pixmap[pos+2] && pixmap[pos+3] && pixmap[pos+2+PIXMAP_W] && pixmap[pos+3+PIXMAP_W] &&
			     pixmap[pos+2+2*PIXMAP_W] && pixmap[pos+3+2*PIXMAP_W] && pixmap[pos+2+3*PIXMAP_W] && pixmap[pos+3+3*PIXMAP_W] &&
			     pixmap[pos + 2*PIXMAP_W] && pixmap[pos + 3*PIXMAP_W] && pixmap[pos+1+2*PIXMAP_W] && pixmap[pos+1+3*PIXMAP_W])
			    result.push_back(Piece {_4x4, x, y});
	    }
	}
	//reverse(result.begin(), result.end());
	return result;
}

vector<bool> insert_piece(const vector<bool>& original, const Piece& p) {
	vector<bool> result = original;

	int pos = p.y * PIXMAP_W + p.x;

	switch (p.type) {
	case _1x8:
		result[pos+7] = false;
		result[pos+6] = false;
	case _1x6:
		result[pos+5] = false;
		result[pos+4] = false;
	case _1x4:
		result[pos+3] = false;
	case _1x3:
		result[pos+2] = false;
	case _1x2:
		result[pos+1] = false;
	case _1x1:
		result[pos] = false;
		break;
	case _8x1:
		result[pos+7*PIXMAP_W] = false;
		result[pos+6*PIXMAP_W] = false;
	case _6x1:
		result[pos+5*PIXMAP_W] = false;
		result[pos+4*PIXMAP_W] = false;
	case _4x1:
		result[pos+3*PIXMAP_W] = false;
	case _3x1:
		result[pos+2*PIXMAP_W] = false;
	case _2x1:
		result[pos+1*PIXMAP_W] = false;
		result[pos] = false;
		break;
	case _2x2:
		result[pos+PIXMAP_W] = false;
		result[pos+1+PIXMAP_W] = false;
		result[pos] = false;
		result[pos+1] = false;
		break;
	case _4x4:
		result[pos+3*PIXMAP_W] = false;
		result[pos+1+3*PIXMAP_W] = false;
		result[pos+2+3*PIXMAP_W] = false;
		result[pos+3+3*PIXMAP_W] = false;
		result[pos+2*PIXMAP_W] = false;
		result[pos+1+2*PIXMAP_W] = false;
		result[pos+2+2*PIXMAP_W] = false;
		result[pos+3+2*PIXMAP_W] = false;
		result[pos+PIXMAP_W] = false;
		result[pos+1+PIXMAP_W] = false;
		result[pos+2+PIXMAP_W] = false;
		result[pos+3+PIXMAP_W] = false;
		result[pos] = false;
		result[pos+1] = false;
		result[pos+2] = false;
		result[pos+3] = false;
		break;
	}

	return result;
};

vector<Piece> solve(const vector<bool>& pixmap, int min_val_per_tile) {
	vector<Element> elements {Element {pixmap, count(cbegin(pixmap), cend(pixmap), true), Piece {}, 0, true, -1}};
	auto comp = [&](unsigned i1, unsigned i2) -> bool {
	    const auto& e1 = elements[i1];
	    const auto& e2 = elements[i2];
	    int v1 = e1.value + e1.open_tiles * min_val_per_tile;
	    int v2 = e2.value + e2.open_tiles * min_val_per_tile;
	    if (v1 == v2)
	        return i1 > i2;
	    return v1 > v2;
	};
    vector<unsigned> open_list {0};
    make_heap(open_list.begin(), open_list.end(), comp);

	for(;;) {
	    int parent_idx = open_list.front();
//		int parent_idx = lowest_value_open(elements);

		pop_heap(open_list.begin(), open_list.end(), comp);
		open_list.pop_back();

		if (is_solved(elements[parent_idx])) {
			vector<Piece> result;
			const Element *elt = &elements[parent_idx];
			while(elt->parent_idx >= 0) {
				result.push_back(elt->piece);
				elt = &elements[elt->parent_idx];	
			}
//			cerr << "Value: " << elements[parent_idx].value << endl;
			return result;
		}

		int x, y;
		tie(y, x) = get_next_free_position(elements[parent_idx].pixmap);
		for(Piece p : fitting_pieces(elements[parent_idx].pixmap, x, y)) {
			Element e;
			e.piece = p;
			e.pixmap = insert_piece(elements[parent_idx].pixmap, p);
			e.open_tiles = elements[parent_idx].open_tiles - tsize[p.type];
			e.value = elements[parent_idx].value + values[p.type];
			e.parent_idx = parent_idx;
			e.open = true;
			elements.push_back(e);
			open_list.push_back(elements.size()-1);
			push_heap(open_list.begin(), open_list.end(), comp);
		}
		elements[parent_idx].open = false;
		elements[parent_idx].pixmap.clear();
		elements[parent_idx].pixmap.shrink_to_fit();
//		cerr << elements.size() << endl;
	}
}

void print(const Piece& p) {
	cout << names[p.type] << " (" << p.y << "|" << p.x << ")" << endl;
}

void print_ps_header() {
    cout << 
"%!PS-Adobe-3.0 EPSF-3.0" << endl <<
"%%BoundingBox: 0 -" << 10*PIXMAP_H << " " << 10*PIXMAP_W << " 0" << endl <<
R"(1 -1 scale
/B {
7 dict begin
/b exch def
/g exch def
/r exch def
/w exch def
/h exch def
/y exch def
/x exch def
gsave
x 10 mul y 10 mul translate
newpath
2 2 moveto
10 w mul 2 lineto
10 w mul 10 h mul lineto
2 10 h mul lineto
closepath
gsave
r g b setrgbcolor
fill
grestore
stroke
grestore
end
} def)" << endl;
}

void print_ps(const vector<Piece>& v, const Color& col) {
    for (const auto& p : v) {
        cout << p.x << " " << p.y << " " << names[p.type] << " "
             << (col.r/255.0) << " " << (col.g/255.0) << " " << (col.b/255.0) << " B" << endl;
    }
}

unordered_set<Color> get_color_palette(const SDL_Surface *img) {
    unordered_set<Color> result;
    
    int bpp = img->format->BytesPerPixel;
    
    if (bpp != 3)
        throw string {"unsupported color depth in source image"};
    
    for (int x = 0; x < PIXMAP_W; ++x) {
        for (int y = 0; y < PIXMAP_H; ++y) {
            const uint8_t* pixel = static_cast<const uint8_t*>(img->pixels) + y*img->pitch + x*bpp;
            result.insert(*reinterpret_cast<const Color*>(pixel));
        }
    }
    
    return result;
}

vector<pair<Color, vector<bool>>> pixmap_from_image(string filename) {
    SDL_Surface *img = IMG_Load(filename.c_str());
    vector<pair<Color, vector<bool>>> pixmap;
    
    int bpp = img->format->BytesPerPixel;
    
    if (bpp != 3)
        throw string {"unsupported color depth in source image"};

    auto palette = get_color_palette(img);
    
    for(Color current_col : palette) {
        pixmap.push_back(make_pair(current_col, vector<bool>(PIXMAP_W * PIXMAP_H, false)));
        for (int x = 0; x < PIXMAP_W; ++x) {
            for (int y = 0; y < PIXMAP_H; ++y) {
                uint8_t* pixel = static_cast<uint8_t*>(img->pixels) + y*img->pitch + x*bpp;
                Color col = *reinterpret_cast<Color*>(pixel);
                pixmap.back().second[y*PIXMAP_W + x] = (col == current_col);
            }
        }
    }
    
    SDL_FreeSurface(img);
    return pixmap;
}

vector<Piece> optimize_solution(vector<Piece> solution) {
    for(int px = 0; px < PIXMAP_W; px += 4)
        for(int py = 0; py < PIXMAP_H; py += 4) {
            vector<bool> pixmap(PIXMAP_W * PIXMAP_H, true);
            vector<Piece> result;

            for (const Piece& p : solution) {
                if (p.x >= px && p.x < px+8 && p.y >= py && p.y < py+8) {
                    pixmap = insert_piece(pixmap, p);
                } else {
                    result.push_back(p);
                }
            }

            pixmap.flip();

            auto tmp = solve(pixmap, min_val_per_tile_opt);

            result.insert(result.end(), tmp.begin(), tmp.end());
            solution = result;
        }

    return solution;
}

void list_pieces(vector<Piece> solution) {
    int num[] = {0, 0, 0, 0, 0, 0, 0};
    
    for(const Piece& p : solution) {
        switch(p.type) {
        case _1x1:
            num[0]++;
            break;
        case _1x2:
            num[1]++;
            break;
        case _2x1:
            num[1]++;
            break;
        case _3x1:
            num[2]++;
            break;
        case _1x3:
            num[2]++;
            break;
        case _4x1:
            num[3]++;
            break;
        case _1x4:
            num[3]++;
            break;
        case _6x1:
            num[4]++;
            break;
        case _1x6:
            num[4]++;
            break;
        case _8x1:
            num[5]++;
            break;
        case _1x8:
            num[5]++;
            break;
        case _2x2:
            num[6]++;
            break;
        case _4x4:
            num[6]+=4;
            break;
        }
    }

    cerr << "1x1 #" << num[0] << " -> €" << prices[0] * num[0] * 0.01 << endl;
    cerr << "1x2 #" << num[1] << " -> €" << prices[1] * num[1] * 0.01 << endl;
    cerr << "1x3 #" << num[2] << " -> €" << prices[2] * num[2] * 0.01 << endl;
    cerr << "1x4 #" << num[3] << " -> €" << prices[3] * num[3] * 0.01 << endl;
    cerr << "1x6 #" << num[4] << " -> €" << prices[4] * num[4] * 0.01 << endl;
    cerr << "1x8 #" << num[5] << " -> €" << prices[5] * num[5] * 0.01 << endl;
    cerr << "2x2 #" << num[6] << " -> €" << prices[6] * num[6] * 0.01 << endl;
}

int main(int argc, char *argv[]) {
	vector<pair<Color, vector<bool>>> pixmap = pixmap_from_image("lego.png");

    print_ps_header();
    for (const auto& elt : pixmap) {
        cerr << "Color = [" << (int)elt.first.r << ", " << (int)elt.first.g << ", " << (int)elt.first.b << "]" << endl;
	    vector<Piece> solution = solve(elt.second, min_val_per_tile);
	    solution = optimize_solution(solution);
	    print_ps(solution, elt.first);
	    list_pieces(solution);
	}
}
