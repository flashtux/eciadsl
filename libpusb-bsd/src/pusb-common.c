/*
  Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation: 10/10/2003
*/

#include "pusb.h"

#include <stdio.h>

pusb_device_t pusb_search_open(int vendorID, int productID)
{
    pusb_enum_t en;

    if (!pusb_enum_init(&en))
        return NULL;

    while (pusb_enum_next(&en))
    {
        if (pusb_get_device_vendor(&en) == vendorID
            && pusb_get_device_product(&en) == productID)
        {
            pusb_device_t result;

            result = pusb_device_open(&en);
            pusb_enum_done(&en);

            return result;
        }
    }

    pusb_enum_done(&en);
    return NULL;
}
