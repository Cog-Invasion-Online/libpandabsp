#include "c_baseplayer.h"
#include "cl_rendersystem.h"
#include "cl_entitymanager.h"
#include "cl_netinterface.h"
#include "cl_bsploader.h"
#include "clientbase.h"
#include "physicssystem.h"
#include "masks.h"

C_BasePlayer::C_BasePlayer() :
	C_BaseCombatCharacter(),
	CBasePlayerShared(),
	_iv_view_offset( "C_BasePlayer_view_offset" )
{
	add_var( &_view_offset, &_iv_view_offset, LATCH_SIMULATION_VAR );

	_tickbase = 0;
}

void C_BasePlayer::simulate()
{
	std::cout << "C_BasePlayer: tickbase = " << _tickbase << std::endl;

	if ( is_local_player() )
	{
		clnet->cmd_tick();
	}
}

void C_BasePlayer::setup_controller()
{
	PhysicsSystem *phys;
	cl->get_game_system_of_type( phys );

	//CBasePlayerShared::setup_controller();
	_controller = new PhysicsCharacterController( clbsp->get_bsp_loader(), phys->get_physics_world(), clrender->get_render(),
						      clrender->get_render(), 4.0f, 2.0f, 0.3f, 1.0f,
						      phys->get_physics_world()->get_gravity()[0],
						      wall_mask, floor_mask, event_mask );
	_np.reparent_to( _controller->get_movement_parent() );
	_np = _controller->get_movement_parent();
	std::cout << "Client controller setup" << std::endl;
	clrender->get_render().ls();
}

void C_BasePlayer::spawn()
{
	BaseClass::spawn();

	setup_controller();

	// Check if we're the local player
	if ( clents->get_local_player_id() == get_entnum() )
	{
		clents->set_local_player( this );
		std::cout << "Set local player!" << std::endl;
	}
}

bool C_BasePlayer::is_local_player() const
{
	return clents->get_local_player() == this;
}

IMPLEMENT_CLIENTCLASS_RT( C_BasePlayer, CBasePlayer )
	RecvPropInt( PROPINFO( _tickbase ) ),
	RecvPropVec3( PROPINFO( _view_offset ) )
END_RECV_TABLE()