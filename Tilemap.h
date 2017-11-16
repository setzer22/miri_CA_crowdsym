#pragma once

#include "rapidxml/rapidxml.hpp"

/*namespace tilemap {


struct Tilemap {
    int num_layers;
    int tilewidth, tileheight;
    int width, height;
};

struct Tileset {
    int firstgid;
    const char* source;
    int width;
    int height;
};

struct Layer {
    int width;
    int height;
    Array<int> data;
};

char* read_file(char* filename) {
    // Note: This only reads ASCII.
    FILE* f = fopen(filename, "r");
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    rewind(f);

    char* buffer = (char*)malloc((size + 1) * sizeof(char));  // TODO: Malloc
    fread(buffer, size, 1, f);
    fclose(f);
    buffer[size] = 0;

    return buffer;
}

namespace xml = rapidxml;
typedef xml::xml_attribute<> attr;
typedef xml::xml_node<> node;

node* maybe_find_unique_child(char* name, node* n) {
    for(node* c = n->first_node(); c; c = c->next_sibling()) {
        if(strcmp(c->name(), name) == 0) {
            return c;
        }
    } 
    return nullptr;
}

node* find_unique_child(char* name, node* n) {
    node* r = maybe_find_unique_child(name, n);
    if(!r) {
    error("Requested for unexisting child:");
    error(name);
    exit(0);
    }
    return r;
}

char* maybe_attribute_str(const char* s, node* n) {
    for(attr* a = n->first_attribute(); a; a = a->next_attribute()) {
        if(strcmp(s, a->name()) == 0) return a->value();
    }; 
    return nullptr;
}

char* attribute_str(const char* s, node* n) {
    char* r = maybe_attribute_str(s, n);
    if(!r) {
        error("Requested for unexisting attribute:");
        error(s);
        exit(0);
    }
    return r;
}

int attribute_int(const char* s, node* n) {
    return atoi(attribute_str(s, n));
}

int count_child(const char* name, node* n) {
    int count = 0;
    for(node* c = n->first_node(); c; c = c->next_sibling()) {
        if(strcmp(c->name(), name) == 0) {
            ++count;
        }
    }; 
    return count;
};

template<typename Func> void for_all_child(char* name, node* n, Func f) {
    for(node* c = n->first_node(); c; c = c->next_sibling()) {
        if(strcmp(c->name(), name) == 0) {
            f(c);
        }
    }; 
}

template<typename Func> void for_all_child_with_index(char* name, node* n, Func f) {
    int i = 0;
    for(node* c = n->first_node(); c; c = c->next_sibling()) {
        if(strcmp(c->name(), name) == 0) {
            f(c, i);
            ++i;
        }
    }; 
}

Tileset parse_tileset(char* filename) {
    char* tileset_string = read_file(filename);
    xml::xml_document<> doc;
    doc.parse<0>(tileset_string);
    node* root = doc.first_node();

    Tileset t;

    t.width = attribute_int("tilewidth", root);
    t.height = attribute_int("tileheight", root);
    t.source = attribute_str("source", find_unique_child("image", root));

    return t;
}

Array<int> data_node_to_gid_buffer(node* data_node) {
    int num_tiles = count_child("tile", data_node);
    Array<int> tiles = alloc_array<int>(num_tiles);
    for_all_child_with_index("tile", data_node,
            [&tiles](node* t, int i){
                tiles[i] = attribute_int("gid", t);      
            });
    return tiles;
}

Tilemap parse_tilemap(char* filename) {
    char* tilemap_string = read_file(filename);
    xml::xml_document<> doc;
    doc.parse<0>(tilemap_string);

    node* root = doc.first_node();
    Tilemap t; 

    t.tilewidth = attribute_int("tilewidth", root);
    t.tileheight = attribute_int("tileheight", root);
    t.width = attribute_int("width", root);
    t.height = attribute_int("height", root);

    Array<Tileset> tilesets = alloc_array<Tileset>(count_child("tileset", root));
    for_all_child_with_index("tileset", root, 
            [&tilesets](node* n, int i) {
                Tileset tileset;
                const char* source = maybe_attribute_str("source", n);
                if (source == nullptr) {
                    node* image_node = find_unique_child("image", n);
                    tileset.source = attribute_str("source", image_node);
                    tileset.width = attribute_int("width", image_node); 
                    tileset.height = attribute_int("height", image_node); 
                } else {
                    tileset = parse_tileset(const_cast<char*>(source));
                }
                tileset.firstgid = attribute_int("firstgid", n);
                tilesets[i] = tileset;
            });

    Array<Layer> layers = alloc_array<Layer>(count_child("layer", root));
    for_all_child_with_index("layer", root, 
            [&layers](node* n, int i) {
                Layer layer;
                layer.width = attribute_int("width", n);
                layer.height = attribute_int("height", n);
                layer.data = data_node_to_gid_buffer(find_unique_child("data", n));
                layers[i] = layer;
            });

}
}
*/
