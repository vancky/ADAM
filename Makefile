CC=gcc

CFLAGS=-O2 -std=c99  -fopenmp  -march=native  
LDLIBS=-lm  -lreflapacke -lreflapack -latlas -latlcblas -fopenmp  wcstools-3.9.2/libwcs/libwcs.a
LIBS=phase_function.o read_occ.o Fit_Occ.o calculate_OCs.o find_chord.o  SH.o generate_sphere.o fit_subdiv_model_to_LC_AO.o parse_ini.o calculate_lcs.o Calc_FTC.o calculate_lcurve.o dihedral_angle.o dihedral_angle_reg.o utils.o convex_reg.o area_reg.o sqrt3_subdiv.o  triangulate_sphere.o matrix_ops.o rotate.o is_in_triangle.o FacetsOverHorizon.o FindActualBlockers.o Calculate_AOs.o Calculate_AO.o calc_image_fft_sinc.o read_ephm_data.o process_ao_images.o octantoid_reg.o octantoid_to_trimesh.o fit_oct_model_to_LC_AO.o process_rd_images.o calc_image_fft_unnormed.o Calculate_RDs.o Calculate_Range_Doppler.o readfits_rd.o kiss_fftndr.o kiss_fft.o kiss_fftr.o kiss_fftnd.o readfits.o

adam:	adam.c $(LIBS)
	$(CC) $(CFLAGS)    adam.c $(LIBS)  $(LDLIBS) -Iiniparser/src -Liniparser -liniparser -o adam
	
thermal:	thermal.c $(LIBS)
	$(CC) $(CFLAGS) thermal.c Calculate_Temp.c $(LIBS) $(LDLIBS)  -Iiniparser/src -Liniparser -liniparser -o thermal
libs:	$(LIBS)

kiss_fftndr.o:	Kissfft/tools/kiss_fftndr.c
	$(CC) $(CFLAGS) -c $<

kiss_fft.o:	Kissfft/kiss_fft.c
	$(CC) $(CFLAGS) -c $<

kiss_fftr.o:	Kissfft/tools/kiss_fftr.c
	$(CC) $(CFLAGS) -c $<

kiss_fftnd.o:	Kissfft/tools/kiss_fftnd.c
	$(CC) $(CFLAGS) -c $<


%.o:	%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o 

