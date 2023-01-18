/*
 * gap_height.h
 *
 * Authors: Dario Jimenez
 *
 * @file contains high level data structures to model gap height sensors
 */

#ifndef GUADALOOP_LIB_GAP_HEIGHT_H_
#define GUADALOOP_LIB_GAP_HEIGHT_H_

/*
 * typedef for gapheight distance
 */
typedef int gapheight_dist_t;

/*
 * enumaration for gap height locations on pod
 */
typedef enum gapheight_location {
    SOME_LOC,
    //TODO: change to correct locations
}gapheight_loc_t;

/*
 * struct for gap height sensor
 */
typedef struct gapheight_sensor {
    gapheight_dist_t distance;
    gapheight_loc_t location;
}gapheight_sensor_t;


#endif /* GUADALOOP_LIB_SENSORS_GAP_HEIGHT_H_ */
