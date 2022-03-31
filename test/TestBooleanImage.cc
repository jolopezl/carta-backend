/* This file is part of the CARTA Image Viewer: https://github.com/CARTAvis/carta-backend
   Copyright 2018, 2019, 2020, 2021 Academia Sinica Institute of Astronomy and Astrophysics (ASIAA),
   Associated Universities, Inc. (AUI) and the Inter-University Institute for Data Intensive Astronomy (IDIA)
   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <gtest/gtest.h>

#include "CommonTestUtilities.h"
#include "FileList/FileExtInfoLoader.h"
#include "FileList/FileInfoLoader.h"
#include "Session/Session.h"
#include "Util/Message.h"

using namespace carta;

static const std::string SAMPLE_FILES_PATH = (TestRoot() / "data" / "images" / "casa").string();

class BooleanImageTest : public ::testing::Test {
public:
    static void CheckFileInfoLoader(const std::string& request_filename) {
        std::string filename = SAMPLE_FILES_PATH + "/" + request_filename;
        casacore::ImageOpener::ImageTypes image_type = casacore::ImageOpener::imageType(filename);
        if (image_type == ImageOpener::AIPSPP) {
            casacore::DataType pixel_type = casacore::imagePixelType(filename);
            if (pixel_type == casacore::TpBool) {
                casacore::PagedImage<casacore::Bool> image(filename); ////
                std::cerr << "imageType() = " << image.imageType() << "\n";
                auto coord = image.coordinates();
                auto shape = image.shape();
                auto shape_vec = shape.asStdVector();
                for (int i = 0; i < shape_vec.size(); ++i) {
                    std::cerr << "shape[" << i << "] = " << shape_vec[i] << "\n";
                }
                casacore::IPosition start(image.shape().size());
                start = 0;
                casacore::IPosition end(image.shape());
                end -= 1;
                casacore::Slicer section(start, end, casacore::Slicer::endIsLast);
                casacore::Array<casacore::Bool> tmp(section.length());
                casacore::SubImage<casacore::Bool> sub_image(image, section);
                casacore::RO_MaskedLatticeIterator<casacore::Bool> lattice_iter(sub_image);

                for (lattice_iter.reset(); !lattice_iter.atEnd(); ++lattice_iter) {
                    casacore::Array<casacore::Bool> cursor_data = lattice_iter.cursor();

                    if (image.isMasked()) {
                        casacore::Array<casacore::Bool> masked_data(cursor_data); // reference the same storage
                        const casacore::Array<bool> cursor_mask = lattice_iter.getMask();

                        // Apply cursor mask to cursor data: set masked values to NaN. booleans are used to delete copy of data if necessary
                        bool del_mask_ptr;
                        const bool* cursor_mask_ptr = cursor_mask.getStorage(del_mask_ptr);

                        bool del_data_ptr;
                        casacore::Bool* masked_data_ptr = masked_data.getStorage(del_data_ptr);

                        for (size_t i = 0; i < cursor_data.nelements(); ++i) {
                            if (!cursor_mask_ptr[i]) {
                                masked_data_ptr[i] = NAN;
                            }
                        }

                        // free storage for cursor arrays
                        cursor_mask.freeStorage(cursor_mask_ptr, del_mask_ptr);
                        masked_data.putStorage(masked_data_ptr, del_data_ptr);
                    }

                    casacore::IPosition cursor_shape(lattice_iter.cursorShape());
                    casacore::IPosition cursor_position(lattice_iter.position());
                    casacore::Slicer cursor_slicer(cursor_position, cursor_shape); // where to put the data
                    tmp(cursor_slicer) = cursor_data;
                }

                auto data = tmp.tovector();
                std::vector<float> data_to_bool(data.begin(), data.end());
                for (int i = 0; i < data_to_bool.size(); ++i) {
                    if (data_to_bool[i] == 1.0) {
                        //std::cerr << "data_to_bool[" << i << "] = " << data_to_bool[i] << "\n";
                    }
                }
            }
        }
    }
};

TEST_F(BooleanImageTest, CasaFile) {
    CheckFileInfoLoader("boolean.image");
}
