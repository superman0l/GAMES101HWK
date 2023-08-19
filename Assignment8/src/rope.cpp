#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.

//        Comment-in this part when you implement the constructor
//        for (auto &i : pinned_nodes) {
//            masses[i]->pinned = true;
//        }
	for(int i=0;i<num_nodes;i++){
            masses.push_back(new Mass(((float)(num_nodes-i)/num_nodes)*start+((float)(i)/num_nodes)*end,
            node_mass,false));
        }
        for(int i=0;i<num_nodes-1;i++){
            springs.push_back(new Spring(masses[i],masses[i+1],k));
        }
//        Comment-in this part when you implement the constructor
        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D dir = (s->m2->position-s->m1->position).unit();
            float f = s->k*((s->m2->position-s->m1->position).norm()-s->rest_length);
            s->m1->forces+=f*dir;
            s->m2->forces+=-f*dir;

            Vector2D dampforcetom2 = -0.05*dot(dir, s->m2->velocity-s->m1->velocity)*dir;
            Vector2D dampforcetom1 = -0.05*dot(-dir, s->m1->velocity-s->m2->velocity)*-dir;
            s->m1->forces+=dampforcetom1;s->m2->forces+=dampforcetom2;

            s->m1->forces+=s->m1->velocity*-0.005;
            s->m2->forces+=s->m2->velocity*-0.005;      
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces+=m->mass*gravity;
                
                //m->position += delta_t*m->velocity;
                //m->velocity += delta_t*m->forces/m->mass;//explicit

                m->velocity += delta_t*m->forces/m->mass;
                m->position += delta_t*m->velocity;//semi-
                // TODO (Part 2): Add global damping
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D dir = (s->m2->position-s->m1->position).unit();
            float f = s->k*((s->m2->position-s->m1->position).norm()-s->rest_length);
            s->m1->forces+=f*dir+s->m1->mass*gravity;
            s->m2->forces+=-f*dir+s->m2->mass*gravity;
            //s->m1->forces+=s->m1->velocity*-0.0005;
            //s->m2->forces+=s->m2->velocity*-0.0005;   
            //Vector2D dampforcetom2 = -0.05*dot(dir, s->m2->velocity-s->m1->velocity)*dir;
            //Vector2D dampforcetom1 = -0.05*dot(-dir, s->m1->velocity-s->m2->velocity)*-dir;
            //s->m1->forces+=dampforcetom1;s->m2->forces+=dampforcetom2;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                m->position += (1-0.00005)*(m->position-m->last_position) + m->forces/m->mass*powf(delta_t,2);
                m->last_position = temp_position;
                // TODO (Part 4): Add global Verlet damping
            }
            m->forces = Vector2D(0, 0);
        }
    }
}