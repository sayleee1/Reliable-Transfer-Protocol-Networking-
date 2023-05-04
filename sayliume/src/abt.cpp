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
struct ABTProtocol
{
  unsigned int acknowledgement_number;
  unsigned int sequence_number = 0;
  unsigned int a_sequence_number, b_sequence_number;
};

list<pkt> packet_list;
ABTProtocol abt_protocol;
struct pkt new_packet;

unsigned int calculate_checksum(struct pkt);

struct pkt get_packet(struct msg message)
{
  struct pkt packet;
  packet.seqnum = abt_protocol.sequence_number;
  packet.acknum = abt_protocol.sequence_number;
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
  abt_protocol.sequence_number++;
  if (abt_protocol.acknowledgement_number == 1)
  {
    abt_protocol.acknowledgement_number = 0;
    list<pkt>::iterator itr = packet_list.begin();
    advance(itr, abt_protocol.a_sequence_number);
    new_packet = *itr;
    tolayer3(0, new_packet);
    starttimer(0, 25.0);
  }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  if (packet.acknum == abt_protocol.a_sequence_number)
  {
    abt_protocol.acknowledgement_number = 1;
    stoptimer(0);
    abt_protocol.a_sequence_number++;
  }
  else
  {
    starttimer(0, 25.0);
    tolayer3(0, new_packet);
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  starttimer(0, 25.0);
  tolayer3(0, new_packet);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  abt_protocol.acknowledgement_number = 1;
  abt_protocol.a_sequence_number = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  if (calculate_checksum(packet) == packet.checksum)
  {
    pkt *received_packet = new struct pkt;
    received_packet->checksum = packet.seqnum;
    if (abt_protocol.b_sequence_number == packet.seqnum)
    {
      tolayer5(1, packet.payload);
      received_packet->acknum = abt_protocol.b_sequence_number;
      abt_protocol.b_sequence_number++;
    }
    else
    {
      received_packet->acknum = -1;
    }
    tolayer3(1, *received_packet);
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  abt_protocol.b_sequence_number = 0;
}