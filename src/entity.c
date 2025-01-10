#include "entity.h"

void EntityManager_Create(arena* Arena, entity_manager* Manager, u32 MaxEntityCount, u32 ComponentCount, ...)
{
  Assert(ComponentCount <= 31 && "To many components!");
  va_list ComponentSizes;
  va_start(ComponentSizes, ComponentCount);

  u32 EntitySize           = 0;
  Manager->ComponentSize   = Arena_Allocate(Arena, sizeof(u32) * ComponentCount);
  Manager->ComponentOffset = Arena_Allocate(Arena, sizeof(u32) * ComponentCount);

  for (u32 ComponentIndex = 0; ComponentIndex < ComponentCount; ComponentIndex++)
  {

    u32 ComponentSize                        = va_arg(ComponentSizes, u32);
    Manager->ComponentSize[ComponentIndex]   = ComponentSize;
    Manager->ComponentOffset[ComponentIndex] = EntitySize;
    EntitySize += ComponentSize;
  }

  Manager->Masks          = Arena_Allocate(Arena, sizeof(u32) * MaxEntityCount);
  Manager->EntityFreeList = Arena_Allocate(Arena, sizeof(u32) * MaxEntityCount);
  for (u32 i = 0; i < MaxEntityCount; i++)
  {
    Manager->EntityFreeList[i] = MaxEntityCount - 1 - i;
  }
  Manager->EntitySize     = EntitySize;
  Manager->Data           = Arena_Allocate(Arena, Manager->EntitySize * MaxEntityCount);
  Manager->EntityCount    = 0;
  Manager->MaxEntityCount = MaxEntityCount;

  Manager->Result.Count   = 0;
  Manager->Result.Ids     = Arena_Allocate(Arena, sizeof(u32) * MaxEntityCount);
}

query_result EntityManager_Query(entity_manager* Manager, u32 ComponentMask)
{
  Manager->Result.Count = 0;
  for (u32 EntityIndex = 0; EntityIndex < Manager->MaxEntityCount; EntityIndex++)
  {

    u32 EntityMask = Manager->Masks[EntityIndex];
    if ((EntityMask & ComponentMask) == ComponentMask && (EntityMask & (1 << 31)))
    {
      Manager->Result.Ids[Manager->Result.Count++] = EntityIndex;
    }
  }

  return Manager->Result;
}

void* EntityManager_GetComponentFromEntity(entity_manager* Manager, entity Entity, u32 ComponentId)
{
  u32  EntityOffset    = Entity * Manager->EntitySize;
  u32  ComponentOffset = Manager->ComponentOffset[ComponentId];

  bool HasComponent    = ((1 << ComponentId) | Manager->Masks[Entity]) > 0;

  return HasComponent ? (u8*)Manager->Data + EntityOffset + ComponentOffset : 0;
}

// ... is just void * pointers to the components layed out by the mask
void EntityManager_AddComponents(entity_manager* Manager, entity Entity, u32 ComponentMask, u32 NumberOfComponents, ...)
{

  va_list ComponentList;
  va_start(ComponentList, NumberOfComponents);
  for (u32 ComponentIndex = 0; ComponentIndex < 31; ComponentIndex++)
  {
    bool HasComponent = ComponentMask & (1 << ComponentIndex);
    if (HasComponent)
    {

      u8* ComponentData   = va_arg(ComponentList, u8*);
      u32 ComponentOffset = Manager->ComponentOffset[ComponentIndex];
      u32 ComponentSize   = Manager->ComponentSize[ComponentIndex];
      u32 EntityOffset    = Manager->EntitySize * Entity;
      u8* EntityData      = (u8*)Manager->Data + EntityOffset + ComponentOffset;
      Memcpy(EntityData, ComponentData, ComponentSize);
    }
  }
  va_end(ComponentList);
}

entity EntityManager_Create_Entity(entity_manager* Manager, u32 ComponentMask)
{
  if (Manager->EntityCount + 1 > Manager->MaxEntityCount)
  {
    Assert(0 && "To many entities!");
  }



  // Get an entity out of the free list
  ++Manager->EntityCount;
  u32    EntityIndex = Manager->MaxEntityCount - Manager->EntityCount;
  entity Entity      = Manager->EntityFreeList[EntityIndex];
  Assert((Manager->Masks[Entity] & (1 << 31)) == 0);

  // Set it's mask bit as alive
  Manager->Masks[Entity] = (1 << 31) | ComponentMask;

  return Entity;
}

void EntityManager_Remove_Entity(entity_manager* Manager, entity Entity, const char * Cause)
{
  Assert(Manager->EntityCount > 1 && "Can't remove the only entity? xD");



  u32 EntityIndex                      = Manager->MaxEntityCount - Manager->EntityCount;

  Manager->EntityFreeList[EntityIndex] = Entity;
  Manager->EntityCount--;
  Manager->Masks[Entity] = 0;
}

void EntityManager_Remove_Components(entity_manager* Manager, entity Entity, u32 ComponentMask)
{
  Manager->Masks[Entity] ^= ComponentMask;
}
