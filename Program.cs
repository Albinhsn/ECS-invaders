using SFML.Window;
using SFML.Graphics;
using SFML.System;

public class GameState
{

}

public class Input
{

}

public class Renderer
{
  // retrieve position and sprite
  // render all the things

}

public class Physics
{
  // retrieve every unit with position and physics
  // update their position
  //  retrieve enemies with reflect thingy
  
  // send collision detection queries for the units and resolve
  //  take damage 

}


public class EnemyManager
{
  // Spawn enemies
  // Shoot bullets
}

public class CollisionDetection
{

}

public struct HealthComponent
{
  int hp;
  bool damageTaken;
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

public struct RenderComponent
{
  Texture texture;
  float alpha;
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

public class Program
{
  public static int Main(string[] args)
  {

    return 0;
  }
}
