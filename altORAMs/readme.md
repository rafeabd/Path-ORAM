# Alternative Approaches

## Overview
This folder contains two alternative versions of the two approaches, which were created in development and we have decided to preserve for your curiosity.

## path_oram
This is simply a path but it doesn't operate on disc, this runs using the exact same method as path_oram_disc.

## rORAM
This was a earlier experiment with rORAM in which we performed something we called chunked evictions, in theory we wanted to speed up the way buckets are processed by not having to access them purely level by level. However, this is rather inaccruate as it is a bit strange in the sense that it essentially reverses the order in which buckets are processed, going from leaf to root and so diverges from the paper. This is purely an experiment and was an attempt to prioritize efficiency over obliviousness, and in our testing outperformed the normal rORAM up until when rORAM passes Path ORAM in efficiency, where it then is mostly on par if not a bit slower.