using SFML.Window;
using SFML.Graphics;
using SFML.System;


public struct HealthComponent
{
  int hp;
}
public struct PositionComponent
{
  float x;
  float y;
}

public struct InputComponent
{
  Keyboard.Key[] keysPressed;
  Keyboard.Key[] keysReleased;
}

public struct SpriteComponent
{
  Texture texture;

}
public struct VelocityComponent
{
  float x;
  float y;
}
public struct ColliderComponent
{
  float width;
  float height;
}

// need a way to query for entities that have certain components 
public struct QueryResult
{

}

public class ECS
{


  public QueryResult Query(int count, params int[] components)
  {
    return new();
  }

}

public abstract class Entity
{
  public abstract void Create(ECS ecs);
}

public class Program
{
  public static int Main(string[] args)
  {

    return 0;
  }
}
