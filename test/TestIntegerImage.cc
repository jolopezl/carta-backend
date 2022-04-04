/* This file is part of the CARTA Image Viewer: https://github.com/CARTAvis/carta-backend
   Copyright 2018, 2019, 2020, 2021 Academia Sinica Institute of Astronomy and Astrophysics (ASIAA),
   Associated Universities, Inc. (AUI) and the Inter-University Institute for Data Intensive Astronomy (IDIA)
   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <gtest/gtest.h>

#include "CommonTestUtilities.h"
#include "FileList/FileExtInfoLoader.h"
#include "FileList/FileInfoLoader.h"
#include "ImageData/CartaCasaImage.h"
#include "Session/Session.h"
#include "Util/Message.h"

using namespace carta;

static const std::string SAMPLE_FILES_PATH = (TestRoot() / "data" / "images" / "casa").string();

class IntegerImageTest : public ::testing::Test {
public:
    static void CheckFile(const std::string& request_filename, std::vector<float>& results) {
        std::string filename = SAMPLE_FILES_PATH + "/" + request_filename;
        casacore::ImageOpener::ImageTypes image_type = casacore::ImageOpener::imageType(filename);
        if (image_type == casacore::ImageOpener::AIPSPP) {
            casacore::DataType pixel_type = casacore::imagePixelType(filename);
            if (pixel_type == casacore::TpInt) {
                casacore::PagedImage<casacore::Int> image(filename);
                casacore::DataType data_type = image.dataType();
                auto coord = image.coordinates();
                auto shape = image.shape();
                auto shape_vec = shape.asStdVector();
                auto nice_cursor_shape = image.doNiceCursorShape(image.advisedMaxPixels());
                auto nice_cursor_shape_vec = nice_cursor_shape.asStdVector();

                casacore::IPosition start(image.shape().size());
                start = 0;
                casacore::IPosition end(image.shape());
                end -= 1;
                casacore::Slicer section(start, end, casacore::Slicer::endIsLast);
                casacore::Array<float> data(section.length());
                casacore::SubImage<casacore::Int> sub_image(image, section);
                casacore::RO_MaskedLatticeIterator<casacore::Int> lattice_iter(sub_image);

                for (lattice_iter.reset(); !lattice_iter.atEnd(); ++lattice_iter) {
                    casacore::Array<float> cursor_data = (const casacore::Array<float>&)lattice_iter.cursor();
                    if (image.isMasked()) {
                        casacore::Array<float> masked_data(cursor_data);
                        const casacore::Array<bool> cursor_mask = lattice_iter.getMask();
                        bool del_mask_ptr;
                        const bool* cursor_mask_ptr = cursor_mask.getStorage(del_mask_ptr);
                        bool del_data_ptr;
                        float* masked_data_ptr = masked_data.getStorage(del_data_ptr);
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
                    casacore::Slicer cursor_slicer(cursor_position, cursor_shape);
                    data(cursor_slicer) = cursor_data;
                }
                results = data.tovector();
                EXPECT_EQ(results.size(), shape.product());
                for (int i = 0; i < results.size(); ++i) {
                    if (results[i] > 0.0) {
                        std::cerr << "results[" << i << "] = " << results[i] << "\n";
                    }
                }
            }
        }
    }

    static void CheckFile2(const std::string& request_filename, std::vector<float>& results) {
        std::string filename = SAMPLE_FILES_PATH + "/" + request_filename;
        casacore::ImageOpener::ImageTypes image_type = casacore::ImageOpener::imageType(filename);
        if (image_type == casacore::ImageOpener::AIPSPP) {
            casacore::DataType pixel_type = casacore::imagePixelType(filename);
            if (pixel_type == casacore::TpInt) {
                carta::CartaCasaImage<casacore::Int> image(filename);
                casacore::DataType data_type = image.dataType();
                auto coord = image.coordinates();
                auto shape = image.shape();
                auto shape_vec = shape.asStdVector();
                auto nice_cursor_shape = image.doNiceCursorShape(image.advisedMaxPixels());
                auto nice_cursor_shape_vec = nice_cursor_shape.asStdVector();

                casacore::IPosition start(image.shape().size());
                start = 0;
                casacore::IPosition end(image.shape());
                end -= 1;
                casacore::Slicer section(start, end, casacore::Slicer::endIsLast);
                casacore::Array<float> data(section.length());
                casacore::SubImage<float> sub_image(image, section);
                casacore::RO_MaskedLatticeIterator<float> lattice_iter(sub_image);

                for (lattice_iter.reset(); !lattice_iter.atEnd(); ++lattice_iter) {
                    casacore::Array<float> cursor_data = lattice_iter.cursor();
                    if (image.isMasked()) {
                        casacore::Array<float> masked_data(cursor_data);
                        const casacore::Array<bool> cursor_mask = lattice_iter.getMask();
                        bool del_mask_ptr;
                        const bool* cursor_mask_ptr = cursor_mask.getStorage(del_mask_ptr);
                        bool del_data_ptr;
                        float* masked_data_ptr = masked_data.getStorage(del_data_ptr);
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
                    casacore::Slicer cursor_slicer(cursor_position, cursor_shape);
                    data(cursor_slicer) = cursor_data;
                }
                results = data.tovector();
                EXPECT_EQ(results.size(), shape.product());
                for (int i = 0; i < results.size(); ++i) {
                    if (results[i] > 0.0) {
                        std::cerr << "results[" << i << "] = " << results[i] << "\n";
                    }
                }
            }
        }
    }
};

TEST_F(IntegerImageTest, CasaFile) {
    std::vector<float> results1, results2;
    CheckFile("integer.image", results1);
    CheckFile2("integer.image", results2);
    CmpVectors(results1, results2);
}
