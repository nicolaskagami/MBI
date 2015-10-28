
#include "MBI.h"

bool Point::operator== (Point b)
{ 
    if((x == b.x)&&(y == b.y))
        return true; 
    else 
        return false;
}

MBI::MBI(unsigned v, unsigned e)
{
    edges = (EDGE*) malloc(sizeof(EDGE)*e);
    vertices = (VERT*) malloc(sizeof(VERT)*v);
    num_edges = e;
    num_vertices = v;
    if(edges==NULL || vertices==NULL)
    {
        std::cerr << "Memory Allocation Error\n" ;
        exit(1);
    }
    for(unsigned i=0;i<num_vertices;i++)
    {
	    vertices[i].delay = 0;
	    vertices[i].positive_targets = 0;
	    vertices[i].negative_targets = 0;
    }
}
MBI::~MBI()
{
    free(vertices);
    free(edges);
}
void MBI::preallocate(unsigned src,unsigned tgt,bool signal)
{
    if(src>=num_vertices)
    {
        printf("Invalid vertex\n");
        exit(1);
    }
    if(signal)
        vertices[src].positive_targets++;
    else
        vertices[src].negative_targets++;
}
void MBI::indexify()
{
    vertices[0].pindex = 0;
    vertices[0].nindex = vertices[0].positive_targets;
    for(unsigned i = 0; i<(num_vertices-1);i++)
    {
        vertices[i+1].pindex = vertices[i].nindex + vertices[i].negative_targets;
        vertices[i+1].nindex = vertices[i+1].pindex + vertices[i+1].positive_targets;

        vertices[i].positive_targets =0;
        vertices[i].negative_targets =0;
    }
    vertices[num_vertices-1].positive_targets=0; 
    vertices[num_vertices-1].negative_targets=0; 
}
void MBI::add_edge(unsigned src,unsigned tgt,bool signal)
{
    //printf("Adding Edge %d %d \n",src, tgt);
    if(src>=num_vertices)
    {
        printf("Invalid vertex\n");
        exit(1);
    }
    unsigned ind;
    if(signal)
    {
        ind = vertices[src].pindex + vertices[src].positive_targets;
        vertices[src].positive_targets++;
    }
    else
    {
        ind = vertices[src].nindex + vertices[src].negative_targets;
        vertices[src].negative_targets++;
    }
    if(ind>=num_edges)
    {
        printf("Invalid Edge : %u/%d\n",ind, num_edges);
        exit(1);
    }
    edges[ind].target = tgt;    
}
void MBI::set_delay(unsigned vert,unsigned delay)
{
    if(vert>=num_vertices)
    {
        printf("Invalid vertex\n");
        exit(1);
    }
    vertices[vert].delay = delay;
}
void MBI::print()
{
    printf("Forward Star Graph\n");
    printf("Vertices: %d\n",num_vertices);
    printf("Edges: %d\n",num_edges);
    for(unsigned i = 0;i<num_vertices;i++)
    {
        //if((vertices[i].positive_targets+vertices[i].negative_targets)>0)
        {
            printf("Vert %d\n",i);
	    printf("Estimated Delay: %d\n",vertices[i].delay);
            printf("Positive Consumers: ");
            for(unsigned b = vertices[i].pindex,j=0;j<vertices[i].positive_targets;j++)
                printf("%d ",edges[b+j].target);
            printf("\nNegative Consumers: ");
            for(unsigned b = vertices[i].nindex,j=0;j<vertices[i].negative_targets;j++)
                printf("%d ",edges[b+j].target);
            printf("\n");
        }
    }
}

unsigned minHeight(unsigned posConsumers,unsigned negConsumers,unsigned fanout)
{
    unsigned posAvailable = fanout;
    unsigned negAvailable = 0;
    unsigned height = 0;
    if((negConsumers == 0)&&(posConsumers<=fanout)) 
    {
        return height; 
    }
    while((negConsumers<negAvailable)&&(posConsumers<posAvailable)) 
    {
        height++;
        if(height%2)
        {
            //New layer is even (positive)
            posConsumers=negAvailable*fanout;
        }
        else
        {
            //New layer is odd (negative)
            negConsumers=posAvailable*fanout;
        }
    }
    return height;
}

//Fanout limitation algorithms:
//Option 1:
//Add delays one by one, maintain optimality (possibly ordered high to low)
//Init: All with height 0 (invalid)
//Insertion:  
//  If there is space in current layer, 
//      add it there
//  else
//      add it in the next layer
void MBI::option1(unsigned vert)
{
    max_inv_fanout = 2;
    max_cell_fanout = 2;
    unsigned height = minHeight(vertices[vert].positive_targets,vertices[vert].negative_targets,max_inv_fanout);
    unsigned maximum_delay = 0;
    LEVEL * levels = (LEVEL*) malloc(sizeof(LEVEL)*height);
    for(unsigned i=1;i<height;i++)
    {
	    levels[i].vacant=0;
	    levels[i].inv_taken=0;
	    levels[i].signal_taken=0;
    }
    levels[0].vacant = max_cell_fanout;
    unsigned base = vertices[vert].pindex;
    for(unsigned i=0;i<vertices[vert].positive_targets+vertices[vert].negative_targets;i++)
    {
        //Add each target
        for(unsigned h=0;h<height;h++)
        {
            if(levels[h].vacant>0)
            {
                edges[base+i].level = h;
                levels[h].vacant--;
                break;
            }
        }	
    }
}

//Option 2:
//Start with all consumers in the min height and trade your way to optimality
//Init: All with height = min height to represent all
//Trades: Trade F positions in Nth layer for 1 position in (N-1)th layer

int main(int argc, char ** argv)
{
    MBI nets(10,10);
    nets.preallocate(0,1,0);
    nets.preallocate(0,2,1);
    nets.preallocate(0,3,0);
    nets.preallocate(0,4,1);
    nets.preallocate(0,5,0);
    nets.preallocate(0,6,1);
    nets.preallocate(0,7,0);
    nets.preallocate(0,8,1);
    nets.preallocate(0,9,1);
    nets.indexify();
    nets.add_edge(0,1,0); 
    nets.add_edge(0,2,1);
    nets.add_edge(0,3,0);
    nets.add_edge(0,4,1);
    nets.add_edge(0,5,0);
    nets.add_edge(0,6,1);
    nets.add_edge(0,7,0);
    nets.add_edge(0,8,1);
    nets.add_edge(0,9,1);
    nets.set_delay(1,1);
    nets.set_delay(2,2);
    nets.set_delay(3,3);
    nets.set_delay(4,4);
    nets.set_delay(5,5);
    nets.set_delay(6,6);
    nets.set_delay(7,7);
    nets.set_delay(8,8);
    nets.set_delay(9,9);
    nets.print();
}
