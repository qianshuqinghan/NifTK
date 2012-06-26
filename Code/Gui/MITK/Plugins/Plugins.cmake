
# Plug-ins must be ordered according to their dependencies

set(PROJECT_PLUGINS
  Plugins/uk.ac.ucl.cmic.midaseditor:ON
  Plugins/uk.ac.ucl.cmic.gui.qt.common:ON
  Plugins/it.unito.cim.intensityprofile:ON  
  Plugins/uk.ac.ucl.cmic.midasmorphologicalsegmentor:ON
  Plugins/uk.ac.ucl.cmic.midasgeneralsegmentor:ON
  Plugins/uk.ac.ucl.cmic.mitksegmentation:ON
  Plugins/uk.ac.ucl.cmic.gui.qt.niftyview:ON
  Plugins/uk.ac.ucl.cmic.midasnavigation:OFF              # Deprecated.
  Plugins/uk.ac.ucl.cmic.niftyreg:OFF                     # Not ready yet.
  Plugins/uk.ac.ucl.cmic.niftyseg:OFF                     # Not ready yet.
  Plugins/uk.ac.ucl.cmic.affinetransform:ON
  Plugins/uk.ac.ucl.cmic.imagelookuptables:ON
  Plugins/uk.ac.ucl.cmic.imagestatistics:ON
  Plugins/uk.ac.ucl.cmic.thumbnail:ON
  Plugins/uk.ac.ucl.cmic.snapshot:ON
  Plugins/uk.ac.ucl.cmic.surgicalguidance:ON
  Plugins/uk.ac.ucl.cmic.surfaceextractor:ON
)
