#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
#include <vhf.h>
#include "Device.h"





typedef struct _DEVICE_CONTEXT {
    VHFHANDLE VhfHandle;  // �洢����HID�豸�ľ��
    // ����������豸��ص�״̬��Ϣ�������ò�������������
} DEVICE_CONTEXT, * PDEVICE_CONTEXT;
// ������ȡ�豸�����ĵĺ�
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

// {685186F1-995E-40E4-BDD4-184723D15E94}
DEFINE_GUID(DEVICEINTERFACE,
    0x685186f1, 0x995e, 0x40e4, 0xbd, 0xd4, 0x18, 0x47, 0x23, 0xd1, 0x5e, 0x94);

// ����ɨ���붨�� (���ֳ���ʾ��)
#define SCAN_CODE_A        0x04
#define SCAN_CODE_B        0x05
#define SCAN_CODE_C        0x06
#define SCAN_CODE_D        0x07
#define SCAN_CODE_E        0x08
#define SCAN_CODE_F        0x09
#define SCAN_CODE_G        0x0A
#define SCAN_CODE_H        0x0B
#define SCAN_CODE_I        0x0C
#define SCAN_CODE_J        0x0D
#define SCAN_CODE_K        0x0E
#define SCAN_CODE_L        0x0F
#define SCAN_CODE_M        0x10
#define SCAN_CODE_N        0x11
#define SCAN_CODE_O        0x12
#define SCAN_CODE_P        0x13
#define SCAN_CODE_Q        0x14
#define SCAN_CODE_R        0x15
#define SCAN_CODE_S        0x16
#define SCAN_CODE_T        0x17
#define SCAN_CODE_U        0x18
#define SCAN_CODE_V        0x19
#define SCAN_CODE_W        0x1A
#define SCAN_CODE_X        0x1B
#define SCAN_CODE_Y        0x1C
#define SCAN_CODE_Z        0x1D

// ���μ�����
#define MODIFIER_LEFT_CTRL  0x01 // ��Ctrl��������Ƽ���
#define MODIFIER_LEFT_SHIFT 0x02 // ��Shift�����󻻵�����
#define MODIFIER_LEFT_ALT   0x04 // ��Alt�������������Macϵͳ�ж�ӦOption����
#define MODIFIER_LEFT_GUI   0x08 // ��GUI������Windowsϵͳ��ΪWindows������Macϵͳ��ΪCommand����
#define MODIFIER_RIGHT_CTRL 0x10 // ��Ctrl�����ҿ��Ƽ���
#define MODIFIER_RIGHT_SHIFT 0x20 // ��Shift�����һ�������
#define MODIFIER_RIGHT_ALT  0x40 // ��Alt�����ҽ����������ϵͳ�г�ΪAlt Gr�����������������ַ���
#define MODIFIER_RIGHT_GUI  0x80 // ��GUI�����Ҳ��Windows����Command�������ּ��̿���û�д˼���

const UCHAR g_KeyboardReportDescriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x29, 0x65,        //   Usage Maximum (0x65)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};



NTSTATUS status;


// ����ȫ�ֱ�������һ�Σ������ڴ棬�ɳ�ʼ��
WDFDEVICE g_Device = NULL;

NTSTATUS EvtDriverDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    UNREFERENCED_PARAMETER(Driver);
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDFDEVICE device;
    WDFQUEUE queue;
    PDEVICE_CONTEXT deviceContext;
    VHF_CONFIG vhfConfig;
    WDF_IO_QUEUE_CONFIG IoConfig;




    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);


    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("VHF-Device: wdf�豸�����豸ʧ�� : 0x%08x\n", status));
        return status;
    }

    deviceContext = DeviceGetContext(device);
    RtlZeroMemory(deviceContext, sizeof(DEVICE_CONTEXT));

    //��ʼ��VHF
    VHF_CONFIG_INIT(&vhfConfig, WdfDeviceWdmGetDeviceObject(device), sizeof(g_KeyboardReportDescriptor), g_KeyboardReportDescriptor);
    //����vhf�豸
    status = VhfCreate(&vhfConfig, &deviceContext->VhfHandle);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("vhf�豸����ʧ�� 0x%08x\n", status));
        return status;
    }
    KdPrint(("vhf�豸�����ɹ�\n"));

    //����vhf�豸
    status = VhfStart(deviceContext->VhfHandle);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("vhf�豸����ʧ�� 0x%08x\n", status));
        return status;
    }
    KdPrint(("vhf�豸�����ɹ�\n"));






    //����IO���к���
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&IoConfig, WdfIoQueueDispatchSequential);
    IoConfig.EvtIoDeviceControl = EvtIoDeviceControl;
    status = WdfIoQueueCreate(device, &IoConfig, WDF_NO_OBJECT_ATTRIBUTES, &queue);
    if (!NT_SUCCESS(status)) {
        KdPrint(("IO���д���ʧ�� 0x%08X\n", status));
    }
    else {
        KdPrint(("IO���д������ 0x%08X\n", status));
    }

    //�����豸�ӿ�
    WdfDeviceCreateDeviceInterface(device, &DEVICEINTERFACE, NULL);

    // ��ȫ�ֱ���g_Device��ֵ
    g_Device = device;
}



