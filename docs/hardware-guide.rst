Hardware Guide
==============

This guide shows and describes the PCBs in the TES Bias System, 
as well as a guide on how to assemble the enclosure.

LNA Bias Card
-------------
Note that both bias cards are reversible. They can be placed in a card slot in either direction.
The LNA Bias card is capable of providing a positive voltage rail (0 to 4.95V)
for the drain of an LNA and negative voltage rail (-4.95 to 0V) for the gate of an LNA.
This is sufficient for both the 4K and 50K LNAs.

.. figure:: /_static/hardware-pictures/LNA_Bias_Card_side_1.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 80%
    :align: center

    LNA Bias Card Side 1.

.. figure:: /_static/hardware-pictures/LNA_Bias_Card_side_2.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 80%
    :align: center

    LNA Bias Card Side 2.

TES Bias Card
-------------
The TES Bias Card is a 20 bit programmable resistor/voltage divider. 
Side 1 has an array of relays that allow the user to control which resistors 
are included in this programmable voltage divider. 
This provides current steps of 20nA and voltage steps of 

.. figure:: /_static/hardware-pictures/TES_Bias_Card_side_1.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

    TES Bias Card Side 1.

.. figure:: /_static/hardware-pictures/TES_Bias_Card_side_2.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

    TES Bias Card Side 2.

DSUB Converter
--------------

.. figure:: /_static/hardware-pictures/dsub_converter.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

    DSUB converter board to for the 50K LNA board output. Converts 37 pin DSUB to a 25 pin DSUB.

Enclosure
---------

This section details how to assemble the enclosure, including the electronics within.
It's important to note that many of these pictures show a mounting plate sitting atop the 
bottom panel - the mounting plate is NOT in the final design. The 
`production files <put link here>`__ will include bottom panel files only, no mounting plate.

Panel 1
~~~~~~~~~~~~

.. figure:: /_static/hardware-pictures/panel_1.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

    Panel 1 contains all data ports and a power switch.

Panel 2
~~~~~~~~~~~~

.. figure:: /_static/hardware-pictures/panel_2.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

    Panel 2 only contains a power switch.

Bottom Panel
~~~~~~~~~~~~
The bottom panel should be manufactured with several types of press fit studs 
(see `Step 1`_):
M3x6mm, M3x10mm, M3x15mm and M4x10mm. If you want the bottom panel to lay truly flat
on a surface, countersink the 5 unthreaded M3 holes on the left and right of the panel,
and use flat head screws instead of pan head screws (M3x4mm). Be sure to countersink from
the side that does not have the studs sticking out. Reference the pictures below in Step 1
to determine the location of these holes (10 total). This guide will reference
the non-countersunk procedure.

Fully Assembled Enclosure
~~~~~~~~~~~~~~~~~~~~~~~~~

The TES Enclosure BOM lists every component needed to create a TES Bias System.

.. figure:: /_static/hardware-pictures/system_top_down.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

		Top down view of the fully assembled TES Bias System.

Step 1
~~~~~~

| Starting with the bottom panel:
| - Place 2mm spacers down onto the M3x10mm and M3x15mm press fit studs.
| - Screw the L-brackets with M3x5mm pan head screws from below.

.. figure:: /_static/hardware-pictures/step_1.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

    Top down view of the Bottom Panel.

.. figure:: /_static/hardware-pictures/step_1(1).png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

    Alternative view of the Bottom Panel.

Step 2
~~~~~~
It's time to place the electronic hardware onto the bottom panel. 
The below pictures illustrate what parts go where. Note that I was 
unable to render the components of the motherboards, power supply and 
line filter, please refer to the `Fully Assembled Enclosure`_ 
image to see what it should look like when the components are placed
properly.

| After Step 1 is complete:
| - Clip the pins of EVERY through-hole component on motherboard and DSUB converter boards without compromising any electrical connections. Note that it is highly suggested to put a 0.2mm thick electrically insulating adhesive pad under every PCB.
| - Place motherboards, `DSUB Converter`_, bus bars, power supply and line filter.
| - Secure every item down with an M3 locknut, except for the bus bars which requires a 94868A038 (M4x6mm female-female) standoff.

.. figure:: /_static/hardware-pictures/step_2.png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

    Electronic hardware installation on the bottom panel.

.. figure:: /_static/hardware-pictures/step_2(2).png
    :alt: A beautiful landscape with mountains and a lake
    :width: 100%
    :align: center

    Alternative view of electronic hardware installation on the bottom panel.



Step 3
~~~~~~
Next, we'll secure the Aluminum corner extrusions (G123020_1000) 
to the bottom panel. Note that the below pictures do not have the 
electronic hardware installed, this is just for visual clarity of 
the current step. See the `Bottom Panel`_ section for a note on how
to have the system lay flat on a surface.

| To secure the corner extrusions to the bottom panel:
| - Double check that every electrical component is secured to the bottom panel, because you will turning the assembly on it's side or upside-down for the next steps.
| - Place 10 M3x4mm screws into the unthreaded M3 holes from the bottom (flat side).
| - From the top, screw the square M3 nuts onto the M3x4mm screws but only for 1-2 threads. Take care to ensure that the nuts are threaded the same amount on every screw.
| - Gently place the assembly upside-down, such that the M3x4mm screws lay flush with the back of the bottom panel.
| - Slide the corner extrusion onto the 5 M3 square nuts
 