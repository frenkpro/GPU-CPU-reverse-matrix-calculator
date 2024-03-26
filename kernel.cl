#pragma OPENCL EXTENSION cl_khr_fp16 : enable

__kernel void identity_matrix(__global const float* B, __global float* C, int n) {
    const int i = get_global_id(0);
    const int j = get_global_id(1);
    const int index = i * n * 2 + j;
    if (j >= n)
        C[index] = (j - n == i) ? 1.0f : 0.0f;

    else if (j < n)
        C[index] = (float)B[i * n + j];
}

__kernel void devide_row_by_oporn(__global float* augmented_matrix, int size, float oporn, int row) {
    const int k = get_global_id(0);
    const int j = get_global_id(1);

    const int index = row * size * 2 + j;
    if (k == 0)
    {
        //printf("\nindex = %d\n arm[index]=%f\n arm/oporn= %f\nk = %d\n\nPivotP= %f\n\n", index, augmented_matrix[index], augmented_matrix[index] /= oporn,k,oporn);
        augmented_matrix[index] /= oporn;
    }

    barrier(CLK_GLOBAL_MEM_FENCE);


    if (k != row) {
        float factor = augmented_matrix[k * 2 * size + row];
        //float temp = augmented_matrix[k * 2 * size + j];
        augmented_matrix[k * 2 * size + j] -= factor * augmented_matrix[row * 2 * size + j];
        //printf("\ntemp = %f, factor = %f, result = %f\n", temp, factor, augmented_matrix[k * 2 * size + j]);
    }
    barrier(CLK_GLOBAL_MEM_FENCE);

}