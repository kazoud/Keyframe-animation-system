Assignment 6: Salah Danial, Karim Dahrouge

Remarks: -Although the codes for init_playback() and end_playback() were provided, we slightly changed the elements to which current_frame points to 
because it was more in line with how we implemented the linked list.

- The code can create, delete and update keyframes as well as display them to the scene. The animation system also works.

-Regarding interpolate, we increment current_frame whenever t is not 0 but alpha is, i.e when t is an integer. This means we know we are dealing with a new keyframes.

-Equalities with floats are replaced with inequalities with tight bounds because we encountered errors due to floating point imprecisions.