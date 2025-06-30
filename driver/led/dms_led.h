#ifndef _DMS_LED_H_
#define _DMS_LED_H_

#define DMS_MAGIC  'l'
#define DMS_MAXNR   6

#define DMS_LED_OFF _IO(DMS_MAGIC, 0)
#define DMS_LED_ON  _IO(DMS_MAGIC, 1)

#endif  /* _DMS_LED_H_ */