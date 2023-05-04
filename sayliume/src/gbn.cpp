#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#include <string.h>
#include <list>

using namespace std;
struct GBNProtocol
{
  unsigned int acknowledgement_number;
  unsigned int sequence_number = 0;
  unsigned int a_sequence_number, b_sequence_number;
  unsigned int newest_sequence_number = 0;
  unsigned int sequence_number_to_transport = 0;
  unsigned int sequence_number_succeeded = 0;
};

list<pkt> packet_list;
GBNProtocol gbn_protocol;
struct pkt new_packet;

unsigned int calculate_checksum(struct pkt);

struct pkt get_packet(struct msg message)
{
  struct pkt packet;
  packet.seqnum = gbn_protocol.sequence_number;
  packet.acknum = gbn_protocol.sequence_number;
  strcpy(packet.payload, message.data);
  packet.checksum = calculate_checksum(packet);
  return packet;
}

unsigned int calculate_checksum(struct pkt packet)
{
  char data[20];
  strcpy(data, packet.payload);
  unsigned int check_sum = 0;
  unsigned int i = 0;
  check_sum += packet.seqnum;
  check_sum += packet.acknum;
  while (i < 20 && data[i] != '\0')
  {
    check_sum += data[i];
    i++;
  }
  return check_sum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
  pkt packet = get_packet(message);
  packet_list.push_back(packet);
  if (gbn_protocol.sequence_number_to_transport == 0)
  {
    list<pkt>::iterator itr = packet_list.begin();
    advance(itr, gbn_protocol.sequence_number);
    new_packet = *itr;
    tolayer3(0, new_packet);
    gbn_protocol.sequence_number++;
    starttimer(0, 25.0);
    gbn_protocol.sequence_number_to_transport++;
  }
  else if (gbn_protocol.sequence_number_to_transport < getwinsize())
  {
    list<pkt>::iterator itr = packet_list.begin();
    advance(itr, gbn_protocol.sequence_number);
    new_packet = *itr;
    tolayer3(0, new_packet);
    gbn_protocol.sequence_number++;
    gbn_protocol.sequence_number_to_transport++;
  }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  gbn_protocol.acknowledgement_number = 1;
  if (packet.acknum == gbn_protocol.newest_sequence_number + 1)
  {
    gbn_protocol.newest_sequence_number++;
  }
  else if (packet.acknum == gbn_protocol.sequence_number_succeeded + getwinsize())
  {
    gbn_protocol.sequence_number_succeeded += getwinsize();
    stoptimer(0);
    gbn_protocol.sequence_number_to_transport = 0;
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  unsigned int i;
  for (i = gbn_protocol.newest_sequence_number;
   i < gbn_protocol.newest_sequence_number + getwinsize() && i < gbn_protocol.sequence_number_to_transport;
   i++)
  {
    list<pkt>::iterator itr = packet_list.begin();
    advance(itr, i);
    new_packet = *itr;
    tolayer3(0, new_packet);
  }
  starttimer(0, 25.0);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  gbn_protocol.acknowledgement_number = 1;
  gbn_protocol.a_sequence_number = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  if (calculate_checksum(packet) == packet.checksum)
  {
    pkt *received_packet = new struct pkt;
    if (gbn_protocol.b_sequence_number == packet.seqnum)
    {
      tolayer5(1, packet.payload);
      (*received_packet).acknum = gbn_protocol.b_sequence_number;
      (*received_packet).checksum = packet.seqnum;
      gbn_protocol.b_sequence_number++;
    }
    else if (gbn_protocol.b_sequence_number != packet.seqnum)
    {
      (*received_packet).acknum = gbn_protocol.b_sequence_number - 1;
      (*received_packet).checksum = packet.seqnum;
    }
    tolayer3(1, *received_packet);
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  gbn_protocol.b_sequence_number = 0;
}
