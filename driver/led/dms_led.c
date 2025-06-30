#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>


#include <linux/major.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "dms_dev.h"
#include "dms_led.h"

#define LED_NAME    "led" // LED_NAME : led

// LED 장치 정보 구조체 
struct dms_led_dev{
    struct gpio_desc *led_gpiod; // LED가 연결된 GPIO 핀을 가리키는 디스크립터 
};

struct dms_led_dev *dev; // 전역 포인터
static struct class *dms_led_class;
static struct device *dms_led_device;

static int dms_led_major = 0, dms_led_minor = 0;
static int result;
static dev_t dms_led_dev;

static struct cdev dms_led_cdev;

static int dms_led_register_cdev(void);

static int dms_led_open(struct inode *inode, struct file *filp);
static int dms_led_release(struct inode *inode, struct file *filp);
static ssize_t dms_led_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static ssize_t dms_led_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t dms_led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

static int dms_led_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "[dms_led] device opened...\n");
    return 0;
}

static int dms_led_release(struct inode *inode, struct file *fp)
{
    printk(KERN_INFO  "[dms_led] device closed...\n");
    return 0;
}

static ssize_t dms_led_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{    
    char data[50] = {0};

    // 버퍼 크기 초과 방지
    if(count > sizeof(data) - 1){
        count = sizeof(data) - 1;
    }

    if (copy_from_user(data, buf, count)){
        pr_err("[dms_led] error copy from user\n");
        return -EFAULT;
    }

    printk(KERN_INFO "[dms_led] data from user: %s : %lu\n", data, count);

    return count;
}

static ssize_t dms_led_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    char data[] = "[dms_led] this is read func...\n";
    size_t datalen = strlen(data);

    // 파일 포인터 위치에 따른 EOF 처리
    if(*f_pos >= datalen){
        return 0;
    }

    if(count > datalen - *f_pos){
        count = datalen - *f_pos;
    }

    if(copy_to_user(buf, data + *f_pos, count)){
        pr_err("[dms_led] error copy to user\n");
        return -EFAULT;
    }

    *f_pos = count;
    return count;
}

// ioctl 호출 시 LED ON/OFF 처리
static ssize_t dms_led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    if(_IOC_TYPE(cmd) != DMS_MAGIC) return -EINVAL;
    if(_IOC_NR(cmd) >= DMS_MAXNR) return -EINVAL;

    switch(cmd){
        case DMS_LED_OFF:
            gpiod_set_value(dev->led_gpiod, 0); // active low
            break;
        
        case DMS_LED_ON:
            gpiod_set_value(dev->led_gpiod, 1); //

            break;
    }

    return 0;

}

// 파일 디스크립터 구조체 등록
struct file_operations dms_led_fops = {
    .open               = dms_led_open,
    .release             = dms_led_release,
    .write              = dms_led_write,
    .read               = dms_led_read,
    .unlocked_ioctl     = dms_led_ioctl,
};

static int dms_led_register_cdev(void)
{
    int error;

    if(dms_led_major){
        // major / minor 번호를 하나의 장치번호로 만듬
        dms_led_dev = MKDEV(dms_led_major, dms_led_minor);
        // 위의 만든 장치번호 범위를 커널에 등록해서 사용 예약 
        error = register_chrdev_region(dms_led_dev, 1, "dms_led");
    }
    else{
        error = alloc_chrdev_region(&dms_led_dev, dms_led_minor, 1, "dms_led");
        dms_led_major = MAJOR(dms_led_dev);
    }

    if(error < 0){
        printk(KERN_WARNING "[dms_led] can't get major %d\n", dms_led_major);
        return error;
    }

    printk(KERN_INFO "[dms_led] major number = %d\n", dms_led_major);

    cdev_init(&dms_led_cdev, &dms_led_fops);
    dms_led_cdev.owner = THIS_MODULE;
    dms_led_cdev.ops = &dms_led_fops;

    // 초기화한 cdev를 커널에 등록 
    error = cdev_add(&dms_led_cdev, dms_led_dev, 1);
    if(error){
        printk(KERN_NOTICE "[dms_led] cdev_add error %d\n", error);
    }
    return 0;
}

// 플랫폼 드라이버 probe 함수
// device tree 기반으로 GPIO 요청하고, cdev emdfhrgksms 플랫폼 드라이버 함수
static int dms_led_probe(struct platform_device *pdev)
{   
    // kalloc (0으로 초기화해서 메모리 할당)
    dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
    if(!dev)    
        return -ENOMEM;

    // Device Tree에서 GPIO 컨트롤러부터 GPIO 핸들러 요청
    dev->led_gpiod = devm_gpiod_get(&pdev->dev, LED_NAME, GPIOD_OUT_LOW);
    if(IS_ERR(dev->led_gpiod)){
        dev_err(&pdev->dev, "Failed to get GPIO descriptor\n");
        return PTR_ERR(dev->led_gpiod);
    }

    // 드라이버 데이터 저장
    platform_set_drvdata(pdev, dev);

    // LED 초기 상태
    gpiod_set_value(dev->led_gpiod, 0); // off
    dev_info(&pdev->dev, "[dms_led] Module is up...\n");

    // 캐릭터 디바이스 등록
    if((result = dms_led_register_cdev()) < 0) {
        return result;
    }

    // 클래스 생성 (udev에 알림)
    dms_led_class = class_create(THIS_MODULE, "dms_led_class");
    if(IS_ERR(dms_led_class)){
        dev_err(&pdev->dev, "Failed to create class\n");
        return PTR_ERR(dms_led_class);
    }

    // 디바이스 생성 -> /dev/dms_led 디바이스 파일 생성
    dms_led_device = device_create(dms_led_class, NULL, dms_led_dev, NULL, "dms_led");
    if(IS_ERR(dms_led_device)){
        class_destroy(dms_led_class);
        dev_err(&pdev->dev, "Failed to create device\n");
        return PTR_ERR(dms_led_device);          
    }

    return 0;

}

// platform 함수 remove 함수
static int dms_led_remove(struct platform_device *pdev)
{
    struct dms_led_dev *dev = platform_get_drvdata(pdev);

    // LED 끄기
    gpiod_set_value(dev->led_gpiod, 1);
    dev_info(&pdev->dev, "[dms_led] Module is down ...\n");

    // 캐릭터 디바이스 해제
    cdev_del(&dms_led_cdev);
    unregister_chrdev_region(dms_led_dev, 0);

    // 디바이스 제거
    device_destroy(dms_led_class, dms_led_dev);
    class_destroy(dms_led_class);

    return 0;
}


static const struct of_device_id dms_led_of_match[] = {
    { .compatible = "rpi,dms_led", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, dms_led_of_match); // 커널 모듈 export

// 플랫폼 드라이버 구조체
static struct platform_driver dms_led_driver = {
    .driver = {
        .name = "dms_led_driver",
        .of_match_table = dms_led_of_match,
    },
    .probe = dms_led_probe,
    .remove = dms_led_remove,
};

// 모듈 등록 매크로
module_platform_driver(dms_led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JUNGHWALEE");
MODULE_DESCRIPTION("DMS LED control driver using GPIO");