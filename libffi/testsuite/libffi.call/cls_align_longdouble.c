/* Area:	ffi_call, closure_call
   Purpose:	Check structure alignment of long double.
   Limitations:	none.
   PR:		none.
   Originator:	<hos@tamanegi.org> 20031203	 */

/* { dg-do run { xfail mips*-*-* arm*-*-* strongarm*-*-* xscale*-*-* } } */
#include "ffitest.h"

typedef struct cls_struct_align {
  unsigned char a;
  long double b;
  unsigned char c;
} cls_struct_align;

cls_struct_align cls_struct_align_fn(struct cls_struct_align a1,
			    struct cls_struct_align a2)
{
  struct cls_struct_align result;

  result.a = a1.a + a2.a;
  result.b = a1.b + a2.b;
  result.c = a1.c + a2.c;

  printf("%d %g %d %d %g %d: %d %g %d\n", a1.a, (double)a1.b, a1.c, a2.a, (double)a2.b, a2.c, result.a, (double)result.b, result.c);

  return  result;
}

static void
cls_struct_align_gn(ffi_cif* cif, void* resp, void** args, void* userdata)
{

  struct cls_struct_align a1, a2;

  a1 = *(struct cls_struct_align*)(args[0]);
  a2 = *(struct cls_struct_align*)(args[1]);

  *(cls_struct_align*)resp = cls_struct_align_fn(a1, a2);
}

int main (void)
{
  ffi_cif cif;
#ifndef USING_MMAP
  static ffi_closure cl;
#endif
  ffi_closure *pcl;
  void* args_dbl[5];
  ffi_type* cls_struct_fields[4];
  ffi_type cls_struct_type;
  ffi_type* dbl_arg_types[5];

#ifdef USING_MMAP
  pcl = allocate_mmap (sizeof(ffi_closure));
#else
  pcl = &cl;
#endif

  cls_struct_type.size = 0;
  cls_struct_type.alignment = 0;
  cls_struct_type.type = FFI_TYPE_STRUCT;
  cls_struct_type.elements = cls_struct_fields;

  struct cls_struct_align g_dbl = { 12, 4951, 127 };
  struct cls_struct_align f_dbl = { 1, 9320, 13 };
  struct cls_struct_align res_dbl;

  cls_struct_fields[0] = &ffi_type_uchar;
  cls_struct_fields[1] = &ffi_type_longdouble;
  cls_struct_fields[2] = &ffi_type_uchar;
  cls_struct_fields[3] = NULL;

  dbl_arg_types[0] = &cls_struct_type;
  dbl_arg_types[1] = &cls_struct_type;
  dbl_arg_types[2] = NULL;

  CHECK(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 2, &cls_struct_type,
		     dbl_arg_types) == FFI_OK);

  args_dbl[0] = &g_dbl;
  args_dbl[1] = &f_dbl;
  args_dbl[2] = NULL;

  ffi_call(&cif, FFI_FN(cls_struct_align_fn), &res_dbl, args_dbl);
  /* { dg-output "12 4951 127 1 9320 13: 13 14271 140" } */
  printf("res: %d %g %d\n", res_dbl.a, (double)res_dbl.b, res_dbl.c);
  /* { dg-output "\nres: 13 14271 140" } */

  CHECK(ffi_prep_closure(pcl, &cif, cls_struct_align_gn, NULL) == FFI_OK);

  res_dbl = ((cls_struct_align(*)(cls_struct_align, cls_struct_align))(pcl))(g_dbl, f_dbl);
  /* { dg-output "\n12 4951 127 1 9320 13: 13 14271 140" } */
  printf("res: %d %g %d\n", res_dbl.a, (double)res_dbl.b, res_dbl.c);
  /* { dg-output "\nres: 13 14271 140" } */

  exit(0);
}
