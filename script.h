#pragma once 

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <iterator>     

#include <glm/glm.hpp>
#include <glm/ext.hpp>


typedef std::vector<glm::mat4> keyframe;       // 4x4 coordinate frames of objects being animated

class Script {
    std::vector<glm::mat4*> scene;               // pointers to objects in scene
    std::list<keyframe> keyframes; 
    std::list<keyframe>::iterator current_frame;

    int current_frame_number;                    // for animation playback

public:
    Script(std::vector<glm::mat4*> objects); 

    void copy_to_scene();                        // copy current keyframe to scene
    void copy_frame_to_scene(keyframe & kf);     // copy a frame to scene
    void add_from_scene();                       // copy current scene to a new keyframe (n)
    void delete_current_frame();                 // delete current frame if it exists
    void update_from_scene();                    // copy current scene to current keyframe (u)
    void advance();                              // advance to next keyframe if possible
    void retreat();                              // retreat to previous keyframe if possible

    void write_script(std::string filename);
    void read_script(std::string filename);
    
    int current_index();                          // index of current frame, for printing
    void print_current();                         // for debugging

    int nkeyframes();                             // number of keyframes in script
    void init_playback();                         // go to first animation frame
    void end_playback();                          // go to last animation frame

    void interpolate_from_current(float alpha);   // 0 <= alpha < 1, copies to scene
    bool interpolate(float t);                    // called from animation/rendering loop

    // interpolate two RBTs represented by glm::mat4s
    static glm::mat4 interpolate(glm::mat4 & first, glm::mat4 & second, float alpha);
    // interpolate two keyframes 
    static keyframe interpolate(keyframe & first, keyframe & second, float alpha);
};
