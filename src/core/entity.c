#include <malloc.h>
#include <string.h>
#include "entity.h"
#include "math/gmath.h"
#include "SDL3/SDL_log.h"

static void entity_update(Entity* entity, float dt)
{
	if (!entity->is_alive) return;

	if (entity->angle < 0) 
		entity->angle += 2 * MATH_PI;
	if (entity->angle >= 2 * MATH_PI) 
		entity->angle -= 2 * MATH_PI;

	if (entity->is_moving) {
		Vec2 dir = gmath_direction(entity->angle);
		Vec2 move = { 0 };

		move.x += dir.x;
		move.y += dir.y;

		move.x -= dir.x;
		move.y -= dir.y;

		move = gmath_vec2_norm(move);

		entity->pos = move;
	}
}

EntityPool* entity_pool_init()
{
	EntityPool* pool = malloc(sizeof(EntityPool));
	if (!pool) {
		SDL_Log("Failed to allocate memory for entity pool");
		return NULL;
	}
	memset(pool, 0, sizeof(EntityPool));
	SDL_Log("Entity pool initialized");
	return pool;
}

bool entity_pool_push(EntityPool* pool, Entity* entity)
{
	if (!pool || pool->size >= ENTITIES_MAX) {
		SDL_Log("Failed to push entity into the pool. Pool is full or uninitialized");
		return false;
	}
	memcpy(&pool->entities[pool->size], entity, sizeof(Entity));
	pool->size++;
	SDL_Log("Entity `type: %d` pushed into entity pool", entity->type);
	return true;
}

Entity* entity_pool_get(EntityPool* pool, u32 index)
{
	if (!pool || index >= ENTITIES_MAX) return NULL;
	return &pool->entities[index];
}

Entity* entity_pool_find(EntityPool* pool, EntityType type)
{
	if (!pool || pool->size == 0) return NULL;
	
	Entity* entity = NULL;
	for (u32 i = 0; i < pool->size; ++i) {
		if (pool->entities[i].type == type) {
			entity = &pool->entities[i];
			break;
		}
	}
	
	return entity;
}

void entity_pool_free(EntityPool* pool)
{
	if (pool) free(pool);
}

void entity_pool_update(EntityPool* pool, float dt)
{
	if (!pool) return;

	for (u32 i = 0; i < pool->size; ++i) {
		Entity* entity = &pool->entities[i];
		entity_update(entity, dt);
	}
}
