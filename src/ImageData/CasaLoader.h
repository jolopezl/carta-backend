/* This file is part of the CARTA Image Viewer: https://github.com/CARTAvis/carta-backend
   Copyright 2018, 2019, 2020, 2021 Academia Sinica Institute of Astronomy and Astrophysics (ASIAA),
   Associated Universities, Inc. (AUI) and the Inter-University Institute for Data Intensive Astronomy (IDIA)
   SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CARTA_BACKEND_IMAGEDATA_CASALOADER_H_
#define CARTA_BACKEND_IMAGEDATA_CASALOADER_H_

#include <casacore/images/Images/PagedImage.h>

#include "FileLoader.h"

namespace carta {

template <typename T>
class CasaLoader : public FileLoader<T> {
public:
    CasaLoader(const std::string& filename);

    void OpenFile(const std::string& hdu) override;
};

template <typename T>
CasaLoader<T>::CasaLoader(const std::string& filename) : FileLoader<float>(filename) {}

template <typename T>
void CasaLoader<T>::OpenFile(const std::string& /*hdu*/) {
    if (!this->_image) {
        this->_image.reset(new casacore::PagedImage<float>(this->_filename));

        if (!this->_image) {
            throw(casacore::AipsError("Error opening image"));
        }

        this->_image_shape = this->_image->shape();
        this->_num_dims = this->_image_shape.size();
        this->_has_pixel_mask = this->_image->hasPixelMask();
        this->_coord_sys = this->_image->coordinates();
    }
}

} // namespace carta

#endif // CARTA_BACKEND_IMAGEDATA_CASALOADER_H_
