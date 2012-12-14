// Version 0.1
#ifndef BLACK_LABEL_RENDERER_RENDERER_HPP
#define BLACK_LABEL_RENDERER_RENDERER_HPP



#define USE_TILED_SHADING
#define USE_TEXTURE_BUFFER



#include <black_label/renderer/camera.hpp>
#include <black_label/renderer/light.hpp>
#include <black_label/renderer/light_grid.hpp>
#include <black_label/renderer/program.hpp>
#include <black_label/renderer/storage/gpu/model.hpp>
#include <black_label/shared_library/utility.hpp>
#include <black_label/renderer/storage/gpu/model.hpp>
#include <black_label/container.hpp>
#include <black_label/utility/log_severity_level.hpp>
#include <black_label/world.hpp>

#include <algorithm>
#include <memory>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem/convenience.hpp>
#include <boost/lockfree/fifo.hpp>
#include <boost/log/sources/severity_logger.hpp>



namespace black_label {
namespace renderer {

class BLACK_LABEL_SHARED_LIBRARY renderer
{

MSVC_PUSH_WARNINGS(4251)
private:
	struct glew_setup { glew_setup(); } glew_setup;
MSVC_POP_WARNINGS()



public:
	typedef world::world world_type;
	typedef world_type::entity_id_type entity_id_type;
	typedef world_type::model_id_type model_id_type;



	renderer( const world_type& world, camera&& camera );
	~renderer();

	void render_frame();
	thread_pool::task render_frame_();

	void report_dirty_model( model_id_type id )
	{ dirty_models.enqueue(id); }
	void report_dirty_model( 
		world_type::model_container::const_iterator model ) 
	{ report_dirty_model(model - world.static_entities.models.cbegin()); }
	void report_dirty_models( 
		world_type::model_container::const_iterator first, 
		world_type::model_container::const_iterator last )
	{ while (first != last) report_dirty_model(first++); }

	void report_dirty_static_entity( entity_id_type id )
	{ dirty_static_entities.enqueue(id); }
	void report_dirty_static_entities( 
		world_type::entities_type::group::const_iterator first, 
		world_type::entities_type::group::const_iterator last )
	{ while (first != last) report_dirty_static_entity(*first++); }

	void report_dirty_dynamic_entity( entity_id_type id )
	{ dirty_dynamic_entities.enqueue(id); }
	void report_dirty_dynamic_entities( 
		world_type::entities_type::group::const_iterator first, 
		world_type::entities_type::group::const_iterator last )
	{ while (first != last) report_dirty_dynamic_entity(*first++); }
	
	void on_window_resized( int width, int height );



	camera camera;



protected:
	renderer( const renderer& other ); // Possible, but do you really want to?



private:
MSVC_PUSH_WARNINGS(4251)

	typedef boost::lockfree::fifo<model_id_type> dirty_model_id_container;
	typedef boost::lockfree::fifo<entity_id_type> dirty_entity_id_container;
	typedef std::unique_ptr<storage::gpu::model[]> model_container;
	typedef container::svector<entity_id_type> sorted_entities_container;
	typedef light_grid::light_container light_container;
	typedef storage::gpu::texture_2d texture;

	void update_lights();
	bool import_model( const boost::filesystem::path& path, storage::gpu::model& gpu_model );
	void import_model( model_id_type entity_id );
	


	boost::log::sources::severity_logger<utility::severity_level> log;

	const world_type& world;

	dirty_model_id_container dirty_models;
	dirty_entity_id_container dirty_static_entities, dirty_dynamic_entities;

	model_container models;
	sorted_entities_container sorted_statics, sorted_dynamics;
	light_container lights;

	light_grid light_grid;

	program buffering, null, lighting, blur_horizontal, blur_vertical, tone_mapper;
	unsigned int framebuffer;
	texture main_render, depths, shadow_map, wc_normals, albedos, 
		bloom1, bloom2, random_texture;
	storage::gpu::texture_buffer<float> gpu_lights;

	glm::mat4 projection_matrix;

MSVC_POP_WARNINGS()
};

} // namespace renderer
} // namespace black_label



#endif
