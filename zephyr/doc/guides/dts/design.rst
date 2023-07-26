.. _dt-design:

Design goals
############

Zephyr's use of devicetree has evolved significantly over time, and further
changes are expected. The following are the general design goals, along with
specific examples about how they impact Zephyr's source code, and areas where
more work remains to be done.

Single source for all hardware information
******************************************

Zephyr shall obtain its hardware descriptions exclusively from devicetree.

Examples
========

- New device drivers shall use devicetree APIs to determine which :ref:`devices
  to create <dt-create-devices>` if possible.

- In-tree sample applications shall use :ref:`aliases <dt-alias-chosen>` to
  determine which of multiple possible generic devices of a given type will be
  used in the current build. For example, the :ref:`blinky-sample` uses this to
  determine the LED to blink.

- Boot-time pin muxing and pin control can be accomplished via devicetree.

Example remaining work
======================

- Zephyr's :ref:`twister_script` currently use :file:`board.yaml` files to
  determine the hardware supported by a board. This should be obtained from
  devicetree instead.

- Various device drivers currently use Kconfig to determine which instances of a
  particular compatible are enabled. This can and should be done with devicetree
  overlays instead.

- Board-level documentation still contains tables of hardware support which are
  generated and maintained by hand. This can and should be obtained from the
  board level devicetree instead.

- Runtime determination of ``struct device`` relationships should be done using
  information obtained from devicetree, e.g. for device power management.

Source compatibility with other operating systems
*************************************************

Zephyr's devicetree tooling is based on a generic layer which is interoperable
with other devicetree users, such as the Linux kernel.

Zephyr's binding language *semantics* can support Zephyr-specific attributes,
but shall not express Zephyr-specific relationships.

Examples
========

- Zephyr's devicetree source parser, :ref:`dtlib.py <dt-scripts>`, is
  source-compatible with other tools like `dtc`_ in both directions:
  :file:`dtlib.py` can parse ``dtc`` output, and ``dtc`` can parse
  :file:`dtlib.py` output.

- Zephyr's "extended dtlib" library, :file:`edtlib.py`, shall not include
  Zephyr-specific features. Its purpose is to provide a higher-level view of the
  devicetree for common elements like interrupts and buses.

  Only the high-level :file:`gen_defines.py` script, which is built on top of
  :file:`edtlib.py`, contains Zephyr-specific knowledge and features.

.. _dtc: https://git.kernel.org/pub/scm/utils/dtc/dtc.git/about/

Example remaining work
======================

- Zephyr has a custom :ref:`dt-bindings` language *syntax*. While Linux's
  dtschema does not yet meet Zephyr's needs, we should try to follow what it is
  capable of representing in Zephyr's own bindings.

- Due to inflexibility in the bindings language, Zephyr cannot support the full
  set of bindings supported by Linux.

- Devicetree source sharing between Zephyr and Linux is not done.
