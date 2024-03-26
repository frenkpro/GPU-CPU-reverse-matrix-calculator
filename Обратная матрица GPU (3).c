#define _CRT_SECURE_NO_DEPRECATE
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <time.h>
#define MAX_SOURCE_SIZE (0x100000)

void checkBuildError(cl_program program, cl_device_id device) {
	cl_build_status build_status;
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);

	if (build_status != CL_SUCCESS) {
		size_t log_size;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		char* log = (char*)malloc(log_size);
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		fprintf(stderr, "Ошибка при сборке программы OpenCL:\n%s\n", log);

		free(log);
		exit(EXIT_FAILURE);
	}
}

int main(void) {
	srand(time(NULL));

	int m, choise;
	printf("Choose way of entering matrix\n1 - Manual\n2 - Random\n:");
	scanf("%d", &choise);
	printf("Enter size of matrix:");
	scanf("%d", &m);

	float* B;

	if (choise == 1)
	{

		B = (float*)malloc(m * m * sizeof(float));
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < m; j++) {
				printf("matrix[%d][%d]=", i + 1, j + 1);
				scanf("%f", &B[i * m + j]);
			}
		}
	}
	else if (choise == 2)
	{
		B = (float*)malloc(m * m * 2 * sizeof(float));
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < m; j++) {
				B[i * m + j] = (float)(rand() % 100);
			}
		}
	}
	else
	{
		printf("ERROR");
		return 0;
	}

	// Load the kernel source code into the array source_str
	FILE* fp;
	char* source_str;
	size_t source_size;
	fp = fopen("kernel.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);


	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_uint nP;
	cl_int ret = clGetPlatformIDs(0, NULL, &nP);
	//printf("Num platforms: %d\n", nP);
	ret = clGetPlatformIDs(nP, &platform_id, NULL);

	char* platformName = NULL;
	size_t nameSize = 0;
	cl_int errPlatformNameSize = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 0, NULL, &nameSize);
	platformName = (char*)malloc(sizeof(char) * nameSize);
	cl_int errPlatformName = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, nameSize, platformName, 0);
	//printf("Platform name: %s\n", platformName);

	cl_uint nD;
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &nD);
	//printf("Num device: %d\n", nD);

	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, nD,
		&device_id, NULL);

	/*Device info*/
	size_t deviceNameSize = 0;
	cl_int errDeviceNameSize = clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &deviceNameSize);

	char* deviceName = (char*)malloc(sizeof(char) * deviceNameSize);
	cl_int errDeviceName = clGetDeviceInfo(device_id, CL_DEVICE_NAME, deviceNameSize, deviceName, NULL);
	printf("Device name: %s\n", deviceName);
	printf("\n");

	cl_context context = clCreateContext(NULL, nD, &device_id, NULL, NULL, &ret);
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	cl_program program = clCreateProgramWithSource(context, 1,
		(const char**)&source_str, (const size_t*)&source_size, &ret);
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	checkBuildError(program, device_id);

	cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
		m * m * 2 * sizeof(cl_float), NULL, &ret);
	cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
		m * m * 2 * sizeof(cl_float), NULL, &ret);

	/*Дополнение единичной матрицей*/
	ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0,
		m * m * 2 * sizeof(float), B, 0, NULL, NULL);
	clock_t start_time = clock();
	cl_kernel kernel = clCreateKernel(program, "identity_matrix", &ret);

	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&b_mem_obj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&c_mem_obj);
	ret = clSetKernelArg(kernel, 2, sizeof(int), (void*)&m);

	size_t global[2] = { m, m * 2 };
	float* C = (float*)malloc(sizeof(float) * m * m * 2);
	cl_event event;
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global, NULL, 0, NULL, &event);
	clWaitForEvents(1, &event);

	cl_event readevent;
	ret = clEnqueueReadBuffer(command_queue, c_mem_obj, 1, 0, m * m * 2 * sizeof(cl_float), C, 0, 0, &readevent);
	clWaitForEvents(1, &readevent);
	kernel = clCreateKernel(program, "devide_row_by_oporn", &ret);
	ret = clEnqueueWriteBuffer(command_queue, c_mem_obj, CL_TRUE, 0,
		m * m * 2 * sizeof(float), C, 0, NULL, NULL);
	
	for (int i = 0; i < m; i++)
	{	
		int index = i * 2 * m + i;
		if (abs(C[index])==0)
		{
			printf("\nThis matrix have no inverse matrix\n");
			exit(0);
		}
		else
		{
			ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&c_mem_obj);
			ret = clSetKernelArg(kernel, 1, sizeof(int), (void*)&m);
			ret = clSetKernelArg(kernel, 2, sizeof(float), (void*)&C[index]);
			ret = clSetKernelArg(kernel, 3, sizeof(int), (void*)&i);

			ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global, NULL, 0, NULL, &event);
			clWaitForEvents(1, &event);
			ret = clEnqueueReadBuffer(command_queue, c_mem_obj, 1, 0, m * m * 2 * sizeof(cl_float), C, 0, 0, &readevent);
			clWaitForEvents(1, &readevent);
		}
	}
	
	/*Приведение к нужному виду*/

	if (m <= 10) {
		for (int i = 0; i < m; i++) {
			for (int j = m; j <2 * m; j++) {
				printf("%0.3f ", C[i*2*m+j]);
			}
			printf("\n");
		}
	}
	else
		{
		printf("Matrix is too big. Written in output.txt\n");
		FILE* f = fopen("output.txt", "w");
		for (int i = 0; i < m; i++) {
			fprintf(f, "\n");
			for (int j = m; j <2 * m; j++) {
				fprintf(f, "%0.3f ", C[i *2* m + j]);
			}
		}
	}

	clock_t end_time = clock(); // конечное время
	double search_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC); // искомое время
	printf("Time %lf seconds\n", search_time);

	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(b_mem_obj);
	ret = clReleaseMemObject(c_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	free(B);
	free(C);
	exit(0);
}