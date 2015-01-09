#include <stdio.h>
#include <signal.h>
#include <arpa/inet.h>
#include <pcap.h>
#include <GeoIP.h>
#include "dict.h"

/****** structs ******/

struct ether_packet {
    u_char ether_src[6];
    u_char ether_dst[6];
    u_short ether_type;
};

struct ip_packet {
    u_char ip_vhl;
    u_char ip_tos;
    u_short ip_len;
    u_short ip_id;
    u_short ip_off;
    u_char ip_ttl;
    u_char ip_pro;
    u_short ip_chk;
    struct in_addr ip_src;
    struct in_addr ip_dst;
};

/****** function prototypes ******/

static void sigint_catch(int signo);
void finish(int n);
pcap_t* pcap_init();
void handle_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

void print_name_table(entry *root);
void print_ea(const u_char *e);

/****** global variables ******/

pcap_t *handle; // our global pcap session handle
GeoIP *gi; // our global GeoIP session handle

dictionary *dict;

/****** function implementations ******/

int main(int argc, char *argv[]) {

    // catch SIGINT so we can close things up properly
    if(signal(SIGINT, sigint_catch) == SIG_ERR) {
        fprintf(stderr, "An error occurred while setting a signal handler.\n");
        exit(EXIT_FAILURE);
    }

    handle = pcap_init();
    gi = GeoIP_open("/usr/share/GeoIP/GeoIP.dat", GEOIP_STANDARD);

    dict = new_dict(225, 0);

    while(1) {
        pcap_loop(handle, 1, handle_packet, NULL);
    }

    /*entry *root;
    unsigned int a, b;

    a = 1853;
    b = 1992;
    root = new_dict(1991, 1);
    print_dict(root);
    insert_entry(root, 1992, 2);
    print_dict(root);
    insert_entry(root, 1993, 3);
    print_dict(root);
    insert_entry(root, 1992, 22);
    print_dict(root);
    close_dict(root);*/

    return EXIT_SUCCESS;
}

static void sigint_catch(int signo) {
    printf("\nSIGINT caught, exiting...\n");
    finish(EXIT_SUCCESS);
}

void finish(int n) {
    if(handle != NULL) {
        pcap_close(handle);
    }
    if(gi != NULL) {
        GeoIP_delete(gi);
    }
    if(dict != NULL) {
        close_dict(dict);
    }
    exit(n);
}

pcap_t* pcap_init() {
    char* dev;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *local_handle;

    struct bpf_program fp;
    char filter_exp[] = "ip";
    bpf_u_int32 net, mask; // IP and netmask of local device

    // ask pcap for a suitable device
    dev = pcap_lookupdev(errbuf);
    if(dev == NULL) {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
        finish(EXIT_FAILURE);
    }

    // open the device
    local_handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if(local_handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        finish(EXIT_FAILURE);
    }

    // ensure we can get ethernet headers
    if(pcap_datalink(local_handle) != DLT_EN10MB) {
        fprintf(stderr, "Device %s doesn't provide Ethernet headers - not supported\n", dev);
        finish(EXIT_FAILURE);
    }

    // grab our IP and netmask
    if(pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
        fprintf(stderr, "Couldn't get IP or netmask for device %s: %s\n", errbuf);
        net = 0;
        mask = 0;
    }

    // compile and apply filter for IP traffic
    if(pcap_compile(local_handle, &fp, filter_exp, 0, mask) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(local_handle));
        finish(EXIT_FAILURE);
    }
    if(pcap_setfilter(local_handle, &fp) == -1) {
        fprintf(stderr, "Couldn't apply filter %s: %s\n", filter_exp, pcap_geterr(local_handle));
        finish(EXIT_FAILURE);
    }

    printf("Listening on %s...\n", dev);
    return local_handle;
}

void handle_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    const struct ether_packet *ether;
    const struct ip_packet *ip;
    int country_id;
    char *country_name;

    ether = (struct ether_packet*)(packet);
    ip = (struct ip_packet*)(packet + 14);  //since ethernet headers are always 14 bytes

    //printf("%s -> ", inet_ntoa(ip->ip_src));
    //printf("%s\n", inet_ntoa(ip->ip_dst));

    //country = GeoIP_country_code_by_addr(gi, inet_ntoa(ip->ip_src));
    country_id = GeoIP_id_by_addr(gi, inet_ntoa(ip->ip_dst));
    //country_name = GeoIP_country_name_by_id(gi, country_id);
    entry *e = find_entry(dict, country_id);
    if(e == NULL) {
        insert_entry(dict, country_id, 1);
    } else {
        e->value++;
    }
    print_name_table(dict);
}

void print_name_table(dictionary *root) {
    entry *e;
    e = root;
    printf("------\n");
    while(e != NULL) {
        printf("%u\t%s\n", e->value, GeoIP_country_name_by_id(gi, e->key));
        e = e->next;
    }
}

void print_ea(const u_char *e) {
    printf("%x:%x:%x:%x:%x:%x", e[0], e[1], e[2], e[3], e[4], e[5]);
}
