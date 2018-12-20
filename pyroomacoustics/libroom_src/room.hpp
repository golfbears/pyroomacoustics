#ifndef __ROOM_H__
#define __ROOM_H__

#include <vector>
#include <stack>
#include <tuple>
#include <Eigen/Dense>
#include <algorithm>

#include "wall.hpp"
#include "utility.hpp"


extern float libroom_eps;

/* The 'entry' type is simply defined as an array of 2 floats.
 * It represents an entry that is logged by the microphone
 * during the ray_tracing execution.
 * The first one of those float will be the travel time of a ray reaching
 * the microphone. The second one will be the energy of this ray.*/
typedef std::array<float,2> entry;
typedef std::list<entry> mic_log;
typedef std::vector<mic_log> room_log;

typedef Eigen::Matrix < int, Eigen::Dynamic, 1 > VectorXi;
typedef Eigen::Matrix < bool, Eigen::Dynamic, Eigen::Dynamic > MatrixXb;
typedef Eigen::Matrix < bool, Eigen::Dynamic, 1 > VectorXb;

struct ImageSource {
  /*
   * A class to hold the information relating to an Image source when running the ISM
   */

  Eigen::VectorXf loc;
  float attenuation;
  int order;
  int gen_wall;
  ImageSource * parent;
  VectorXb visible_mics;
};

/*
 * Structure for a room as a list of walls
 * with a few sources and microphones around
 */
class Room {
  public:
  int dim;
  std::vector <Wall> walls;

  // List of obstructing walls
  std::vector <int> obstructing_walls;

  // The microphones are in the room
  Eigen::MatrixXf microphones;

  // Very useful for raytracing
  // 1. Position format that fits the need of ray_tracing
  uint n_mics;
  // 2. A distance after which a ray must have hit at least 1 wall
  float max_dist;

  // This is a list of image sources
  Eigen::MatrixXf sources;
  VectorXi gen_walls;
  VectorXi orders;
  Eigen::VectorXf attenuations;

  // This array will get filled by visibility status
  // its size is n_microphones * n_sources
  MatrixXb visible_mics;

  // Constructor
  Room() {} // default

  Wall & get_wall(int w) {
    return walls[w];
  }

  // Image source model methods
  int image_source_model(const Eigen::VectorXf & source_location, int max_order);

  // A specialized method for the shoebox room case
  int image_source_shoebox(
    const Eigen::VectorXf & source,
    const Eigen::VectorXf & room_size,
    const Eigen::VectorXf & absorption,
    int max_order);

  float get_max_distance();

  std::tuple < Eigen::VectorXf, int > next_wall_hit(
    const Eigen::VectorXf & start,
    const Eigen::VectorXf & end,
    bool scattered_ray);

  bool scat_ray(
	float energy,
	float scatter_coef,
	const Wall & last_wall,
	const Eigen::VectorXf & start,
	const Eigen::VectorXf & hit_point,
	float radius,
	float total_dist,
	float travel_time,
	float time_thres,
	float energy_thres,
	float sound_speed,
	room_log & output);

  void simul_ray(float init_phi,
    float init_theta,
    const Eigen::VectorXf source_pos,
    float mic_radius,
    float scatter_coef,
    float time_thres,
    float energy_thres,
    float sound_speed,
    room_log & output);

  room_log get_rir_entries(size_t nb_phis,
    size_t nb_thetas,
    const Eigen::VectorXf source_pos,
    float mic_radius,
    float scatter_coef,
    float time_thres,
    float energy_thres,
    float sound_speed);

  bool contains(const Eigen::VectorXf point);

  private:
    // We need a stack to store the image sources during the algorithm
    std::stack < ImageSource > visible_sources;

  // Image source model internal methods
  void image_sources_dfs(ImageSource & is, int max_order);
  bool is_visible_dfs(const Eigen::VectorXf & p, ImageSource & is);
  bool is_obstructed_dfs(const Eigen::VectorXf & p, ImageSource & is);
  int fill_sources();

};


#endif // __ROOM_H__