#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "arm.h"
#include "config.h"
#if 0
#include <byteswap.h>
zed      = 800.0;
shoulder = -90.0;
elbow    = 0.0;
yaw      = 0.0;
pitch    = 0.0;
roll     = 0.0;
grip     = 0.0;
#endif

/* from #include <bits/byteswap.h> @ linux */
#define bswap_constant_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |               \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

	
static char joint_names[NOJOINTS][10] = {
	{ "zed" },
	{ "shoulder" },
	{ "elbow" },
	{ "yaw" },
	{ "pitch" },
	{ "roll" },
	{ "gripper" },
};

void
swap_binary_robot(robot_t *in, robot_t *out)
{
    int i;
    strcpy(out->name, in->name);
    
    for (i = 0; i < NOJOINTS; i++)
    {
#if 0
        out->link[i].theta = bswap_64(in->link[i].theta);
        out->link[i].alpha = bswap_32((float )(in->link[i].alpha));
        out->link[i].a = bswap_constant_32((float )(in->link[i].a));
        out->link[i].d = bswap_constant_32((int )in->link[i].d);
        out->link[i].ECmin = bswap_constant_32((int )in->link[i].ECmin);
        out->link[i].ECmax = bswap_constant_32((int )in->link[i].ECmax);
        out->link[i].Rmin = bswap_constant_32((int )in->link[i].Rmin);
        out->link[i].Rmax = bswap_constant_32((int )in->link[i].Rmax);
        out->link[i].RT = bswap_constant_32((int )in->link[i].RT);
        out->link[i].theta = bswap_constant_32((int )in->link[i].theta);
#else
        out->link[i].alpha = bswap_constant_32((int )in->link[i].alpha);
        out->link[i].a = bswap_constant_32((int )in->link[i].a);
        out->link[i].d = bswap_constant_32((int )in->link[i].d);
        out->link[i].ECmin = bswap_constant_32((int )in->link[i].ECmin);
        out->link[i].ECmax = bswap_constant_32((int )in->link[i].ECmax);
        out->link[i].Rmin = bswap_constant_32((int )in->link[i].Rmin);
        out->link[i].Rmax = bswap_constant_32((int )in->link[i].Rmax);
        out->link[i].RT = bswap_constant_32((int )in->link[i].RT);
#endif
    }
}


void robot_info (r)
robot_t *r;
{
        int i;
    printf("Robotnaam = %s\n", r->name);
    printf("           Theta  alpha       a       d    ecmin    ecmax    Rmin    Rmax    RT\n");

    for (i = 0; i < NOJOINTS; i++)
    {
	printf("%8s:", joint_names[i]);
        printf(" %6.2lf",r->link[i].theta);
        printf(" %6.2lf",r->link[i].alpha);
        printf(" %7.2lf",r->link[i].a    );
        printf(" %7.2lf",r->link[i].d    );
        printf(" %8.2lf",r->link[i].ECmin);
        printf(" %8.2lf",r->link[i].ECmax);
        printf(" %7.2lf",r->link[i].Rmin );
        printf(" %7.2lf",r->link[i].Rmax );
        printf(" %5.2lf",r->link[i].RT   );
        printf("\n");
    }
}

int
write_xml_robot (char *name, robot_t *r)
{
   FILE  *fd;
   int i;

    fd = fopen (name, "w");
    if (fd == NULL)
    {
        perror(name);
        fprintf(stderr,"Sorry, cannot store the lengths of the arm.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    fprintf(fd, "<?xml version='1.0'?>\n");
    fprintf(fd, "<robot>\n");
    fprintf(fd, "<name>'%s'</name>\n", r->name);
    fprintf(fd, "<number_of_links>%d</number_of_links>\n", NOJOINTS);
    /* printf("Theta\talpha\ta\td\tecmin\tecmax\tRmin\tRmax\tRT\n"); */

    for (i = 0; i < NOJOINTS; i++)
    {
        fprintf(fd, "  <link>\n");
	fprintf(fd, "    <name>'%9s'</name>\n", joint_names[i]);
        fprintf(fd, "    <theta>%10.2lf</theta>\n",r->link[i].theta);
        fprintf(fd, "    <alpha>%10.2lf</alpha>\n",r->link[i].alpha);
        fprintf(fd, "    <a>%14.2lf</a>\n",r->link[i].a);
        fprintf(fd, "    <d>%14.2lf</d>\n",r->link[i].d);
        fprintf(fd, "    <ECmin>%10.2lf</ECmin>\n",r->link[i].ECmin);
        fprintf(fd, "    <ECmax>%10.2lf</ECmax>\n",r->link[i].ECmax);
        fprintf(fd, "    <Rmin>%11.2lf</Rmin>\n",r->link[i].Rmin);
        fprintf(fd, "    <Rmax>%11.2lf</Rmax>\n",r->link[i].Rmax);
        fprintf(fd, "    <RT>%13.2lf</RT>\n",r->link[i].RT);
        fprintf(fd, "  </link>\n");
    }
    fprintf(fd, "</robot>\n");
    fclose (fd);

    printf("Written information about robot into file:\n\t'%s'.\n", name);

    return 0;
}


int
read_xml_robot (char *name, robot_t *r)
{
   FILE  *fd;
   int i, items, no_joints;
   float xml_version;
   char tag[50];

    fd = fopen (name, "r");
    if (fd == NULL)
    {
        perror(name);
        fprintf(stderr,"Sorry, cannot read the lengths of the arm.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    items = fscanf(fd, "<?xml version='%f'?>\n",&xml_version);
    items = fscanf(fd, "<%5s>\n",tag);

    if (items != 1 || strncmp (tag, "robot",5) != 0)
    {
        perror(name);
        fprintf(stderr,"Sorry, file doesn't contain expected xml-data.\n");
        fprintf(stderr,"Can't continue.\n");
        fclose (fd);
        return -1;
    }

    items = fscanf(fd, "<name>'%[^']'</name>\n", r->name);
    items = fscanf(fd, "<number_of_links>%d</number_of_links>\n", &no_joints);

    if (items != 1 || no_joints > NOJOINTS)
    {
        perror(name);
        fprintf(stderr,"Sorry, file contains more links than expected\n");
        fprintf(stderr,"Can't continue.\n");
        fclose (fd);
        return -1;
    }

    /* printf("Theta\talpha\ta\td\tecmin\tecmax\tRmin\tRmax\tRT\n"); */

    for (i = 0; i < no_joints; i++)
    {
        items = fscanf(fd, "  <%s>\n", tag);
	items = fscanf(fd, "    <name>'%[^']'</name>\n", joint_names[i]);
        items = fscanf(fd, "    <theta>%lf</theta>\n",&(r->link[i].theta));
        items = fscanf(fd, "    <alpha>%lf</alpha>\n",&(r->link[i].alpha));
        items = fscanf(fd, "    <a>%lf</a>\n",&(r->link[i].a));
        items = fscanf(fd, "    <d>%lf</d>\n",&(r->link[i].d));
        items = fscanf(fd, "    <ECmin>%lf</ECmin>\n",&(r->link[i].ECmin));
        items = fscanf(fd, "    <ECmax>%lf</ECmax>\n",&(r->link[i].ECmax));
        items = fscanf(fd, "    <Rmin>%lf</Rmin>\n",&(r->link[i].Rmin));
        items = fscanf(fd, "    <Rmax>%lf</Rmax>\n",&(r->link[i].Rmax));
        items = fscanf(fd, "    <RT>%lf</RT>\n",&(r->link[i].RT));
        items = fscanf(fd, "  <%s>\n",tag);
    }
    items = fscanf(fd, "  <%s>\n",tag);
    fclose (fd);
    printf("Read information about robot from xml-file:\n\t'%s'.\n", name);

    return 0;
}


int
read_binary_robot (char *name, robot_t *robot)
{
    int fd;

    fd = open (name, O_RDWR);
    if (fd < 0)
    {
        perror(name);
        fprintf(stderr,"Sorry, don't know the lengths of the arm.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    if (read (fd, robot, sizeof (robot_t)) < sizeof (robot_t))
    {
        perror ("bad read");
        fprintf(stderr,"Sorry, couldn't read all information for the robot.\n");
        fprintf(stderr,"Maybe I could try to deduce some more information..\n");
        fprintf(stderr,"But quitting is easier.\n");
        return -1;
    }
    close (fd);
    printf("Read information about robot from file:\n\t'%s'.\n", name);

    return 0;
}

int
write_binary_robot (char *name, robot_t *robot)
{
   int fd;

    fd = open (name, O_CREAT, O_RDWR);
    if (fd < 0)
    {
        perror(name);
        fprintf(stderr,"Sorry, cannot store the lengths of the arm.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    if (write (fd, robot, sizeof (robot_t)) < sizeof (robot_t))
    {
        perror ("bad read");
        fprintf(stderr,"Sorry, couldn't write all information for the robot.\n");
        fprintf(stderr,"Maybe I could try to deduce some more information..\n");
        fprintf(stderr,"But quitting is easier.\n");
        return -1;
    }
    close (fd);
    printf("Written information about robot into binary file:\n\t'%s'.\n", name);

    return 0;
}

#ifdef MAIN
int
main()
{
    robot_t robot_solaris;
    robot_t robot_linux;
    int return_value;

#if 0
    read_binary_robot (UMI_RTX, &robot_solaris);
    robot_info(&robot_solaris);
    swap_binary_robot(&robot_solaris, &robot_linux);
    write_binary_robot ("/home/arnoud/src/robotica/data/linux/umi.rtx", &robot_solaris);
    write_xml_robot ("/home/arnoud/src/robotica/data/linux/umi.rtx", &robot_solaris);
#else
    return_value = read_xml_robot (UMI_RTX, &robot_linux);
    if (return_value >= 0)
      write_binary_robot ("../data/linux/umi.rtx", &robot_linux);
#endif
    return return_value;
}

#endif /* MAIN */
