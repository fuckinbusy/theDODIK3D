#ifndef _CORE_ENTITY_H
#define _CORE_ENTITY_H
#include <stdbool.h>
#include "utypes.h"
#include "math/gmath.h"

#define ENTITIES_MAX 20
#define ENTITY_VELOCITY 1.5f

typedef enum EntityType {
	ENTITY_TEST_0 = 100,
} EntityType;

typedef struct Entity {
	Vec2 pos;
	Vec2 dir;

	u32 type;
	u32 health;
	u32 damage;

	float angle;
	float velocity;
	float rot_velocity;
	float radius;

	bool is_shooting;
	bool is_angry;
	bool is_alive;
	bool is_moving;
} Entity;

typedef struct EntityPool {
	Entity entities[ENTITIES_MAX];
	u32 size;
} EntityPool;

static inline Entity entity_create(Vec2 pos, Vec2 dir, u32 health, EntityType type)
{
	Entity ent = { 0 };
	ent.type = type;
	ent.pos = pos;
	ent.dir = dir;
	ent.health = health;
	ent.velocity = ENTITY_VELOCITY;
	ent.rot_velocity = ENTITY_VELOCITY;
	ent.is_alive = health > 0 ? true : false;
	return ent;
}

EntityPool* entity_pool_init();
bool entity_pool_push(EntityPool* pool, Entity *entity);
Entity* entity_pool_get(EntityPool* pool, u32 index);
Entity* entity_pool_find(EntityPool* pool, EntityType id);
void entity_pool_free(EntityPool* pool);

void entity_pool_update(EntityPool* pool, float dt);

#endif // _CORE_ENTITY_H