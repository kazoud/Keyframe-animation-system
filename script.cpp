#pragma once 

#include "script.h"
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>      // rotation, translation, scaling transforms

using namespace std;

Script::Script(std::vector<glm::mat4 *> objects)
{
    for (int i = 0; i < objects.size(); i++)
        scene.push_back(objects[i]);
    current_frame = keyframes.end();
}


// copy current keyframe to scene
void Script::copy_to_scene()
{
    if (keyframes.begin() != keyframes.end())    // if list of keyframes not empty
    {
        for (int i = 0; i < scene.size(); i++)
            *scene[i] = (*current_frame)[i];
    }
    std::cout << "Keyframe copied from keyframe " << current_index() << std::endl; //Added by me for clarity
}

// copy a given frame to scene
void Script::copy_frame_to_scene(keyframe &kf)
{
    for (int i = 0; i < scene.size(); i++)
        *scene[i] = kf[i];
}

// copy current scene to a new keyframe, after the current one (n)
void Script::add_from_scene()
{
    keyframe* NewKeyframe = new keyframe();
    for (int i = 0; i < scene.size(); i++)
    {
        NewKeyframe->push_back(*scene[i]);
    }
    std::cout << "New keyframe created" << endl;

    if (current_frame == keyframes.end())
    {
        keyframes.push_back(*NewKeyframe);
        current_frame--;
    }

    else
    {
        current_frame++;
        keyframes.insert(current_frame, *NewKeyframe);
        current_frame--;
    }

    std::cout << "Keyframe added at position " << current_index() << std::endl;
}


// delete current farme if it exists, and set it to previous one, unless it was first
void Script::delete_current_frame()
{
    if (current_frame != keyframes.end())
    {
        std::list<keyframe>::iterator temp = current_frame;
        if (current_frame == keyframes.begin())
        {
            temp++;
        }
        else
        {
            temp--;
        }

        keyframes.erase(current_frame);
        std::cout << "Deleting the current keyframe. " << std::endl;
        if (keyframes.begin() == keyframes.end())
        {
            current_frame = keyframes.end();
        }
        else
        {
            current_frame = temp;
            std::cout << "Cursor reassigned to keyframe " << current_index() << std::endl;
            this->copy_to_scene();
        }
        
    }
    else
    {
        std::cout << "Current frame is undefined. " << std::endl;
    }
    
}

// copy current scene to current keyframe, if keyframe exists (u)
void Script::update_from_scene()
{
    if (keyframes.begin() != keyframes.end())
    {
        for (int i = 0; i < scene.size(); i++)
        {
            (*current_frame)[i] = *(scene[i]);
        }

        std::cout << "Keyframe " << current_index() << " was updated. " << std::endl;
    }
    else
    {
        this->add_from_scene();
    }
    
    
}

// advance to next key frame if possible
void Script::advance()
{
  
    if (current_frame != keyframes.end())
    {
        current_frame++;
        if (current_frame != keyframes.end())
        {
            std::cout << "Advancing to keyframe " << current_index() << std::endl;
            this->copy_to_scene();
        }   
        else
        {
            std::cout << "You are now at the end of the list. " << std::endl;
        }
    }
    else
    {
        std::cout << "You are already at the last keyframe." << std::endl;
    }
    
    
}

void Script::retreat()
{
    if (current_frame != keyframes.begin())
    {
        current_frame--;
        std::cout << "Retreating to keyframe " << current_index() << std::endl;
        this->copy_to_scene();
    }
    else
    {
        std::cout << "You are already at the first keyframe." << std::endl;
    }
}

void Script::write_script(string filename)
{
    ofstream file;
    file.open(filename);
    for (std::list<keyframe>::iterator it = keyframes.begin(); it != keyframes.end(); ++it)
    {
        for (int i = 0; i < scene.size(); i++)
        {
            float *a = glm::value_ptr(it->at(i));
            for (int k = 0; k < 16; k++)
                file << a[k] << ",";
            file << ' ';   // space to separate object frames
        }
        file << std::endl; // newline to separate keyframes
    }
    file.close();
}

void Script::read_script(string filename)
{
    // TO DO (OPTIONAL) 
}

// index of current frame
int Script::current_index()
{
    if (keyframes.begin() == keyframes.end())
        return -1;
    else
        return std::distance(keyframes.begin(), current_frame); // O(N) algorithm for list!
} 

void Script::print_current()
{
    for (int i = 0; i < scene.size(); i++)
    {
        float *a = glm::value_ptr(current_frame->at(i));
        for (int k = 0; k < 16; k++)
            std::cout << a[k] << ",";
        std::cout << std::endl;
    }
}

int Script::nkeyframes()
{
    return keyframes.size();
}

void Script::init_playback()
{
    //current_frame = std::next(keyframes.begin());
    current_frame = keyframes.begin();
    current_frame_number = 0;
    std::cout << "Starting the animation. " << std::endl;
}

void Script::end_playback()
{
    //current_frame = std::prev(keyframes.end(), 2); // go to last animation frame
    current_frame = std::prev(keyframes.end());
    copy_to_scene();
}

void Script::interpolate_from_current(float alpha)
{
    std::list<keyframe>::iterator next_frame = current_frame;
    assert(std::next(next_frame) != keyframes.end());

    next_frame++;

    keyframe interpolated = interpolate(*current_frame, *next_frame, alpha);
    copy_frame_to_scene(interpolated);

}

bool Script::interpolate(float t)
{
  float alpha = t - glm::floor(t);

  if (nkeyframes() - 1 - t < 0.0001) //We are done with the animation
  {
      return true;
  }

  else if (t != 0.0f && (abs(alpha) < 0.0001 || abs(alpha) > 0.9999)  ) //This means that we crossed a new integer, so we go to the next frame.
  {
      current_frame++;
      interpolate_from_current(alpha);
      std::cout << "pointer incremented " << t << std::endl;
      return false;
  }

  else
  {
      std::cout << "t = " << t << " and alpha = " << alpha << std::endl;
      interpolate_from_current(alpha);
      return false;
  }
}

// interpolate two RBTs represented by glm::mat4s
glm::mat4 Script::interpolate(glm::mat4 &first, glm::mat4 &second, float alpha)
{
    //Translation
    glm::vec3 first_translation = glm::vec3(first[3]);
    glm::vec3 second_translation = glm::vec3(second[3]);

    glm::vec3 lerped = ((1 - alpha) * first_translation) + (alpha * second_translation);
    glm::mat4 translation_RBT = glm::translate(glm::mat4(1.0f), lerped);
   

    //Rotation

    /*glm::quat cube1rot = glm::quat_cast(first);
    glm::quat cube2rot = glm::quat_cast(second);

    glm::quat interpolated_quat = glm::mix(cube1rot, cube2rot, 0.0f);

    if (interpolated_quat == cube1rot)
    {
        std::cout << "Happy carrot." << std::endl;
    }*/

    glm::quat first_rotation = glm::quat(first);
    glm::quat second_rotation = glm::quat(second);
    glm::quat slerped = glm::slerp(first_rotation, second_rotation, alpha);
    glm::mat4 rotation_RBT = glm::mat4(slerped);
    //glm::mat4 rotation_RBT = glm::mat4(first_rotation);
    //Combining
    glm::mat4 interpolated = translation_RBT*rotation_RBT;

    return interpolated; 

}

// interpolate two keyframes
keyframe Script::interpolate(keyframe& first, keyframe& second, float alpha)
{
    //keyframe* interpolated = new keyframe();
    keyframe interpolated;
    for (int i = 0; i < first.size(); i++)
    {
        //interpolated->push_back(interpolate(first[i], second[i], alpha));
        glm::mat4 first_mat = first[i];
        glm::mat4 second_mat = second[i];
        interpolated.push_back(interpolate(first_mat, second_mat, alpha));
    }

    //return *interpolated;
    return interpolated;
}

